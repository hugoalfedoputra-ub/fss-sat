#include "GEOSatelliteCommunications.h"
#include "MissionControlCenter.h"
#include "Tags.h"
#include <fstream>
#include <mutex>

using namespace inet;
using namespace omnetpp;

namespace { // Anonymous namespace
    int packetsLost = 0;
    int packetsSent = 0;
    std::ofstream packetLossOutputFile; // New output file stream
    std::mutex packetLossFileMutex;     // Mutex for new file
}

Define_Module(GEOSatelliteCommunications);

void GEOSatelliteCommunications::initialize()
{
    cBandDownlinkFrequency = par("cBandDownlinkFrequency");
    cBandUplinkFrequency = par("cBandUplinkFrequency");
    noiseFloor_dBm = par("noiseFloor").doubleValue();

    // Register the broadcast signal
    broadcastSignal = registerSignal("broadcastPackets");

    // ... initialize other parameters
    queueProcessingEvent = new cMessage("processQueue");

    packetsLost = 0;
    packetsSent = 0;

    configName = par("configName").stdstringValue();

    // Open the packet loss output file ONLY ONCE and in initialize()
    packetLossFileMutex.lock(); // Corrected: Lock before checking and opening
    if (!packetLossOutputFile.is_open()) {  // Check if already open
        std::string packetLossFilename = configName + "_sat_packet_loss_details.txt";
        packetLossOutputFile.open(packetLossFilename.c_str());
        if (packetLossOutputFile.is_open()) {
            packetLossOutputFile << "simTime,MCC_idx,packet_is_loss" << std::endl;
        } else {
            throw cRuntimeError("Error opening packet loss output file: %s", packetLossFilename.c_str());
        }
    }
    packetLossFileMutex.unlock(); // Unlock after opening

    EV << "GEOSatelliteCommunications initialized with config: " << configName << endl;
    EV << "MissionControlCenter: noiseFloor_dBm = " << noiseFloor_dBm << endl;
    EV << "GEOSatelliteCommunications Initialized " << endl;
}

void GEOSatelliteCommunications::handleMessage(cMessage *msg)
{
    // Check if the message is null
    if (!msg) {
        EV_ERROR << "Received null message" << endl;
        return;
    }

    if (msg->isSelfMessage() && msg == queueProcessingEvent) {
        processQueue();
        return; // Important: Exit after processing the queue
    }

    if (msg->isPacket()) {
        Packet* packet = check_and_cast<Packet*>(msg);
        int senderMCCIndex = -1; // Initialize

        try {
            senderMCCIndex = msg->getArrivalGate()->getIndex();
        } catch (cException& e) {
           EV_ERROR << "Exception: Cannot get index of sender" << endl;
           // Or handle it in other ways
        }

        EV << "Total Latency: " << packet->getTag<LatencyTag>()->getLatency() << " s\n";

        auto powerTag = packet->addTagIfAbsent<PowerTag>();

        // Step 4: Satellite Reception and Received Power Calculation
        double old_power_dBm = powerTag->getReceivedPower_dBm();
        bool packetLostHere = false; // Flag to indicate if the packet was lost at this point

        if (old_power_dBm < noiseFloor_dBm) {
            EV << "Uplink signal below noise floor (" << noiseFloor_dBm << " dBm), PACKET IS LOST AT SAT.\n";
            packetsLost++;
            packetLostHere = true; // Set the flag
            // Write packet loss details *AFTER* checking for loss
            packetLossFileMutex.lock();
            if (packetLossOutputFile.is_open()) {
                packetLossOutputFile << simTime() << "," << senderMCCIndex << "," << (packetLostHere ? 1 : 0)  << std::endl;
            } else {
                EV_ERROR << "Packet loss details file not open!" << std::endl;
            }
            packetLossFileMutex.unlock();
            delete msg;
            return;
        }
        packetsSent++;
        // Write packet loss details *AFTER* checking for loss
        packetLossFileMutex.lock();
        if (packetLossOutputFile.is_open()) {
            packetLossOutputFile << simTime() << "," << senderMCCIndex << "," << (packetLostHere ? 1 : 0)  << std::endl;
        } else {
            EV_ERROR << "Packet loss details file not open!" << std::endl;
        }
        packetLossFileMutex.unlock();

        double eirp_dBm = powerTag->getEIRP_dBm();
        double fspl_dB = powerTag->getFSPL_dB(); // Get FSPL from the tag
        double receiveGain_dBi = getParentModule()->getSubmodule("antenna")->par("gain").doubleValue();  // Satellite receive gain
        double receivedPower_dBm = eirp_dBm - fspl_dB + receiveGain_dBi;

        powerTag->setReceivedPower_dBm(receivedPower_dBm);    // Update received power in the tag
        powerTag->setReceiveGain_dBi(receiveGain_dBi);        // Store Gr_sat

        EV << "Step 4: EIRP: " << eirp_dBm << " dBm\n";
        EV << "Step 4: FSPL: " << fspl_dB << " dB\n";
        EV << "Step 4: Receive Gain (Gr_sat): " << receiveGain_dBi << " dBi\n";
        EV << "Step 4: Received Power (Pr_sat): " << receivedPower_dBm << " dBm\n";

        // Step 5: Satellite Amplification (Example)
        // Rationale for amplification_dB: https://itso.int/wp-content/uploads/2018/04/Presentation-by-Intelsat.pdf
        double amplification_dB = 60.0 - 5.9 + 40.0; // Simplified, supposed to be Tx and Rx
        double amplifiedPower_dBm = receivedPower_dBm + amplification_dB;
        powerTag->setTransmitPower_dBm(amplifiedPower_dBm); // Set this as the new transmit power for downlink

        EV << "Step 5: Amplification: " << amplification_dB << " dB\n";
        EV << "Step 5: Amplified Power: " << amplifiedPower_dBm << " dBm\n";

        //Step 6: Satellite Re-radiation: EIRP Calculation for Downlink
        double transmitGainDownlink_dBi = getParentModule()->getSubmodule("antenna")->par("gain").doubleValue(); // Satellite transmit gain (can be different from receive gain)
        powerTag->setTransmitGain_dBi(transmitGainDownlink_dBi); // Update with downlink transmit gain for clarity
        powerTag->calculateEIRP(); // Recalculate EIRP for downlink using amplified power
        double eirpDownlink_dBm = powerTag->getEIRP_dBm(); // Store newly calculated EIRP for downlink
        powerTag->setEIRP_dBm(eirpDownlink_dBm); // Store in Power Tag
        EV << "Step 6: Transmit Gain (Downlink): " << transmitGainDownlink_dBi << " dBi\n";
        EV << "Step 6: EIRP (Downlink): " << eirpDownlink_dBm << " dBm\n";

        // Log received packet details
        EV << "Satellite received packet: " << packet->getName()
           << " from gate: " << msg->getArrivalGate()->getFullName()
           << " size: " << packet->getByteLength() << " bytes" << endl;

        packetQueue.push(packet);  // Add packet to the queue

        if (!queueProcessingEvent->isScheduled())
            scheduleAt(simTime() + 0.00001, queueProcessingEvent);
    } else {
        EV_WARN << "Received non-packet message, discarding" << endl;
        delete msg;
    }
}

void GEOSatelliteCommunications::processQueue()
{
    if (packetQueue.empty()) {
        EV << "processQueue called but queue is empty." << endl; // New: Log empty queue
        return;
    }

    Packet* packet = packetQueue.front();
    packetQueue.pop();
    try {
       emit(broadcastSignal, packet->getByteLength());

       // 1. Retrieve the target MCC from the packet tag
       int targetMCC = packet->getTag<TargetTag>()->getTarget();
       EV << "Target MCC: " << targetMCC << endl;

       // 2. Find the output gate corresponding to the target MCC
       cGate *outGate = gate("downlinkOut", targetMCC);

       // 3. Forward the packet to the identified output gate
       if (outGate) {
           EV << "Forwarding packet to gate: " << outGate->getName() << " for MCC " << targetMCC << endl; // Corrected line
           send(packet, outGate);
       } else {
           EV_WARN << "No output gate found for target MCC " << targetMCC << ", dropping packet" << endl;
           delete packet;
       }

    } catch (const std::exception& e) {
        EV_ERROR << "Error processing packet: " << e.what() << endl;
        delete packet; // Delete the message in case of error
    }

    if (!packetQueue.empty()) {
         scheduleAt(simTime() + 0.00001, queueProcessingEvent);
    }
}

void GEOSatelliteCommunications::finish()
{
    // Write packet loss data to a file
    std::string filename = configName + "_sat_packet_lost.txt";
    std::ofstream outputFile(filename.c_str());

    if (outputFile.is_open()) {
        outputFile << "Satellite: Packets Sent = " << packetsSent << std::endl;
        outputFile << "Satellite: Packets Lost = " << packetsLost << std::endl;
        outputFile.close();
    } else {
        EV_ERROR << "Could not open file for writing: " << filename << std::endl;
    }
}

// Destructor to close the file (recommended)
GEOSatelliteCommunications::~GEOSatelliteCommunications() {
    packetLossFileMutex.lock();
    if (packetLossOutputFile.is_open()) {
        packetLossOutputFile.close();
    }
    packetLossFileMutex.unlock();
}
