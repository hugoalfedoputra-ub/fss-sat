#include "MissionControlCenter.h"
#include <inet/common/packet/Packet.h>
#include <inet/common/packet/chunk/ByteCountChunk.h>
#include <inet/common/Units.h>
#include <inet/common/INETMath.h>
#include "Tags.h"
#include <random>
#include <fstream>
#include <mutex>

using namespace inet;
using namespace inet::math;

namespace { // Anonymous namespace
    int packetsLost[5]; // Array to store packet loss for each MCC
    int packetsSent[5];
    std::mutex fileMutex;  // Mutex for file access synchronization
    std::ofstream outputFile;
}

Define_Module(MissionControlCenter);

void MissionControlCenter::initialize(int stage)
{
//    EV_INFO << "GEOSatellite::initialize stage " << stage << endl;
    if (stage == 1) {
        iaTime = par("iaTime").doubleValue();
        scheduleAt(simTime() + iaTime, new cMessage("sendMsg"));

        // Initialize random number generator
        rng.seed(time(0) + getIndex()); // Seed based on time and MCC index

        antenna = check_and_cast<GEOSatelliteAntenna*>(getSubmodule("antenna"));
        if (!antenna) {
            throw cRuntimeError("Antenna module not found in MCC");
        }

        packetsLost[getIndex()] = 0; // Initialize packet loss for this MCC
        packetsSent[getIndex()] = 1;

        configName = par("configName").stdstringValue();

        // Open the output file for writing ONLY ONCE and in initialize()
        if (getIndex() == 0) { // Only MCC 0 opens/creates the file
            fileMutex.lock(); // Acquire the lock
            std::string filename = configName + "_mcc_packet_lost.txt";
            outputFile.open(filename.c_str());
            if (!outputFile.is_open()) {
                throw cRuntimeError("Error opening output file: %s", filename.c_str());
            }
            fileMutex.unlock(); // Release the lock
        }

        EV << "MissionControlCenter initialized with config: " << configName << endl;

        noiseFloor_dBm = par("noiseFloor").doubleValue();
        EV << "GEOSatelliteCommunications: noiseFloor_dBm = " << noiseFloor_dBm << endl;

        EV << "MCC " << getIndex() << " iaTime " << iaTime << " Initialized" << endl;
        EV << antenna->getInfo() << endl;
    }
}

void MissionControlCenter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage() && strcmp(msg->getName(), "sendMsg") == 0) {
        auto dataChunk = makeShared<ByteCountChunk>(B(128));
        Packet *packet = new Packet("dataPacket", dataChunk);

        auto powerTag = packet->addTagIfAbsent<PowerTag>();

        // Step 1: Set Transmit Power (Pt)
        double transmitPower_dBm = mW2dBmW(antenna->getPower()); // Assuming antenna power is in mW
        powerTag->setTransmitPower_dBm(transmitPower_dBm);
        EV << "Step 1: Transmit Power (Pt): " << transmitPower_dBm << " dBm\n";

        // Step 2: Calculate EIRP
        double transmitGain_dBi = antenna->getGain();
        powerTag->setTransmitGain_dBi(transmitGain_dBi);
        powerTag->calculateEIRP();
        double eirp_dBm = powerTag->getEIRP_dBm();
        EV << "Step 2: Transmit Gain (Gt): " << transmitGain_dBi << " dBi\n";
        EV << "Step 2: EIRP: " << eirp_dBm << " dBm\n";

        // Determine target MCC (randomly, but not self)
        int targetMCC = getIndex();
        while (targetMCC == getIndex()) {
            targetMCC = std::uniform_int_distribution<int>(0, getParentModule()->par("numOfMCCs").intValue() - 1)(rng);
        }

        // Add target MCC as a tag to the packet
        auto targetTag = packet->addTagIfAbsent<TargetTag>();
        targetTag->setTarget(targetMCC);

        auto mobility = getSubmodule("mobility");
        if (mobility) {
            auto groundStationMobility = dynamic_cast<GroundStationMobility*>(mobility);
            if (groundStationMobility) {
                targetTag->setPosition(groundStationMobility->getRealWorldPosition());
            } else {
                EV_ERROR << "Mobility submodule is not of type LUTMotionMobility" << endl;
            }
        } else {
            EV_ERROR << "Mobility submodule not found" << endl;
        }

        EV << "MCC " << getIndex() << " sending packet to MCC " << targetMCC << endl;

        send(packet, "satOut");
        packetsSent[getIndex()]++;

        scheduleAt(simTime() + iaTime, new cMessage("sendMsg"));
        delete msg;

    } else if (msg->isPacket()) {
        auto *receivedPacket = check_and_cast<Packet*>(msg);

        EV << "Total Latency: " << receivedPacket->getTag<LatencyTag>()->getLatency() << " s\n";

        if (msg->getArrivalGateId() == gate("satIn")->getId()) {
            EV << "MCC " << getIndex() << " received DOWNLINK packet: " << receivedPacket << " from MCC " << receivedPacket->getTag<TargetTag>()->getTarget() <<  endl;

//            auto powerTag = receivedPacket->getTag<PowerTag>();
            auto powerTag = receivedPacket->getTagForUpdate<PowerTag>();

            // Step 7: Calculate received Power
            double old_power_dBm = powerTag->getReceivedPower_dBm();

            if (old_power_dBm < noiseFloor_dBm) {
                EV << "Downlink Signal below noise floor (" << noiseFloor_dBm << " dBm), PACKET IS LOST AT MCC.\n";
                packetsLost[getIndex()]++;
                delete receivedPacket;  // Delete the packet
                return;
            }

            double fspl_atmosphere_db = powerTag->getFSPL_dB();
            double receiveGain_dBi = antenna->getGain(); // Earth Station gain
            double eirp_sat = powerTag->getEIRP_dBm(); // Assuming the satellite transmits with its own EIRP, you might need to fetch this from the packet.

            double receivedPower_dBm = eirp_sat - fspl_atmosphere_db + receiveGain_dBi; // Corrected the order for path loss subtraction

            powerTag->setReceivedPower_dBm(receivedPower_dBm);
            powerTag->setReceiveGain_dBi(receiveGain_dBi);

            EV << "Step 8: Satellite EIRP (EIRP_sat): " << eirp_sat << " dBm\n";
            EV << "Step 8: FSPL: " << fspl_atmosphere_db << " dB\n";
            EV << "Step 8: Receive Gain (Gr_earth): " << receiveGain_dBi << " dBi\n";
            EV << "Step 8: Received Power (Pr_earth): " << receivedPower_dBm << " dBm\n";

        } else {
            EV << "MCC " << getIndex() << " received UPLINK packet " << receivedPacket << endl;
        }
    }
}
void MissionControlCenter::finish()
{
    // Write packet loss statistics to the common file with mutex
    fileMutex.lock(); // Acquire the lock

    if (outputFile.is_open()) {
        outputFile << "MCC " << getIndex() << ": Packets Sent = " << packetsSent[getIndex()] << std::endl;
        outputFile << "MCC " << getIndex() << ": Packets Lost = " << packetsLost[getIndex()] << std::endl;

        if (getIndex() == (getParentModule()->par("numOfMCCs").intValue()-1)) { //Last MCC to close the file
            outputFile.close();
        }
    } else {
        EV_ERROR << "Error: Output file is not open in finish()!" << std::endl;
    }

    fileMutex.unlock(); // Release the lock outside the if condition

    EV << "MCC " << getIndex() << " finished." << endl;
}


