#include "MissionControlCenter.h"
#include <inet/common/packet/Packet.h>
#include <inet/common/packet/chunk/ByteCountChunk.h>
#include <inet/common/Units.h>
#include <inet/common/INETMath.h>
#include "Tags.h"
#include <random>
#include <fstream>
#include <mutex>
#include <filesystem>

using namespace inet;
using namespace inet::math;

namespace { // Anonymous namespace
    int packetsLost[5]; // Array to store packet loss for each MCC
    int packetsSent[5];
    std::mutex fileMutex;  // Mutex for file access synchronization
    std::ofstream outputFile;
    std::ofstream weatherOutputFile; // New output file stream for weather data
    std::mutex weatherFileMutex;     // Mutex for weather file access
    std::ofstream packetLossOutputFile; // New output file stream
    std::mutex packetLossFileMutex;     // Mutex for new file
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

        useSpecDynamicWeather = par("useSpecDynamicWeather").boolValue();
        specWeatherModel = par("specWeatherModel").doubleValue();
        useDynamicCloudCover = par("useDynamicCloudCover").boolValue();

        if (useSpecDynamicWeather) {
            EV << "Using dynamic weather... " << endl;
            // Initialize weather models for each MCC
            std::uniform_real_distribution<double> dist(-5.0, 5.0);
            specWeatherModel = 0.0; // Resetting it just in case
            specWeatherModel += dist(rng);
            if (specWeatherModel < 0.0){
                specWeatherModel = 0.0;
            }
            EV << "Weather model at MCC " << getIndex() << " = " << specWeatherModel << endl;
        } else {
//            specWeatherModel = 0.0;
            // Do nothing and let SCPC handle it
        }

        if (useDynamicCloudCover) {
            if (specWeatherModel < 3.0) {
                cloudCover = 0;
            } else if (3.0 <= specWeatherModel && specWeatherModel < 25.0) {
                cloudCover = 1;
            } else {
                cloudCover = 2;
            }
        } else {
            cloudCover = 0;
        }
        EV << "Cloud cover at MCC " << getIndex() << " = " << cloudCover << endl;

        configName = par("configName").stdstringValue();
        std::string subfolder = "data";

        // Open the packet loss output file for writing ONLY ONCE and in initialize()
        if (getIndex() == 0) { // Only MCC 0 opens/creates the file
            fileMutex.lock(); // Acquire the lock

            std::string filename = subfolder + "/" + configName + "_mcc_packet_lost.txt";

            outputFile.open(filename.c_str());
            if (!outputFile.is_open()) {
                throw cRuntimeError("Error opening output file: %s", filename.c_str());
            }
            fileMutex.unlock(); // Release the lock
        }

        // Open the weather output file ONLY ONCE and in initialize()
        if (getIndex() == 0) { // Only MCC 0 opens/creates the file
            weatherFileMutex.lock();
            std::string weatherFilename = subfolder + "/" + configName + "_mcc_weather_data.txt";
            weatherOutputFile.open(weatherFilename.c_str());
            if (!weatherOutputFile.is_open()) {
                throw cRuntimeError("Error opening weather output file: %s", weatherFilename.c_str());
            }
            // Write header to the weather file
            weatherOutputFile << "simTime,MCCIndex,specWeatherModel,cloudCover" << std::endl;
            weatherFileMutex.unlock();
        }

        // Open the packet loss output file ONLY ONCE and in initialize()
        packetLossFileMutex.lock(); // Corrected: Lock before checking and opening
        if (!packetLossOutputFile.is_open()) {  // Check if already open
            std::string packetLossFilename = subfolder + "/" + configName + "_mcc_packet_loss_details.txt";
            packetLossOutputFile.open(packetLossFilename.c_str());
            if (packetLossOutputFile.is_open()) {
                packetLossOutputFile << "simTime,MCC_idx,packet_is_loss" << std::endl;
            } else {
                throw cRuntimeError("Error opening packet loss output file: %s", packetLossFilename.c_str());
            }
        }
        packetLossFileMutex.unlock(); // Unlock after opening

        EV << "MissionControlCenter initialized with config: " << configName << endl;

        noiseFloor_dBm = par("noiseFloor").doubleValue();
        EV << "GEOSatelliteCommunications: noiseFloor_dBm = " << noiseFloor_dBm << endl;
        EV << "MCC specific weather model = " << specWeatherModel << "mm of rain" << endl;
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

        // Write weather data to file BEFORE weather might have changed (this is uplink)
        weatherFileMutex.lock();
        if (weatherOutputFile.is_open()) {
            weatherOutputFile << simTime() << "," << getIndex() << "," << specWeatherModel << "," << cloudCover << std::endl;
        } else {
            EV_ERROR << "Weather output file is not open!" << std::endl;
        }
        weatherFileMutex.unlock();

        // Chance of changing weather AFTER sending packet
        std::uniform_real_distribution<double> weatherChange(-1.0, 1.0);
        if (weatherChange(rng) > 0.33) { // to be or not to be
            EV << endl << "WEATHER UPDATE !!! WEATHER UPDATE !!! WEATHER UPDATE !!! WEATHER UPDATE !!!"
                    << endl << "Updating weather..." << endl;
            std::uniform_real_distribution<double> dist(-20.0, 15.0);
            specWeatherModel += dist(rng);
            if (specWeatherModel < 0.0){
                specWeatherModel = 0.0;
            }
            if (specWeatherModel > 250.1){
                specWeatherModel /= 3.0;
            }

            if (useDynamicCloudCover) {
                if (specWeatherModel < 3.0) {
                    cloudCover = 0;
                } else if (3.0 <= specWeatherModel && specWeatherModel < 25.0) {
                    cloudCover = 1;
                } else {
                    cloudCover = 2;
                }
            }

            EV << "Weather model at MCC " << getIndex() << " = " << specWeatherModel << " (occurred after sending packet)" << endl;
            EV << "Cloud cover at MCC " << getIndex() << " = " << cloudCover << " (occurred after sending packet)" << endl;
        }

        // Write weather data to file AFTER weather might have changed (this is downlink)
        weatherFileMutex.lock();
        if (weatherOutputFile.is_open()) {
            weatherOutputFile << simTime() + 0.00001 << "," << getIndex() << "," << specWeatherModel << "," << cloudCover << std::endl;
        } else {
            EV_ERROR << "Weather output file is not open!" << std::endl;
        }
        weatherFileMutex.unlock();

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
                // Write packet loss details of lost
                packetLossFileMutex.lock();
                if (packetLossOutputFile.is_open()) {
                    packetLossOutputFile << simTime() << "," << getIndex() << "," << 1 << std::endl;
                } else {
                    EV_ERROR << "Packet loss details file not open!" << std::endl;
                }
                packetLossFileMutex.unlock();
                delete receivedPacket;  // Delete the packet
                return;
            }
            // Write packet loss details of not lost
            packetLossFileMutex.lock();
            if (packetLossOutputFile.is_open()) {
                packetLossOutputFile << simTime() << "," << getIndex() << "," << 0 << std::endl;
            } else {
                EV_ERROR << "Packet loss details file not open!" << std::endl;
            }
            packetLossFileMutex.unlock();

            double fspl_atmosphere_db = powerTag->getPL_dB();
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

MissionControlCenter::~MissionControlCenter() {
    if (getIndex() == 0) { // Only MCC 0 closes the file
        weatherFileMutex.lock();
        if (weatherOutputFile.is_open()) {
            weatherOutputFile.close();
        }
        weatherFileMutex.unlock();
    }

    packetLossFileMutex.lock();
    if (packetLossOutputFile.is_open()) {
        packetLossOutputFile.close();
    }
    packetLossFileMutex.unlock();
}
