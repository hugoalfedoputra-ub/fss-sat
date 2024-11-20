#include <inet/common/ModuleAccess.h>
#include <inet/common/packet/Packet.h>
#include <inet/common/geometry/common/Coord.h>
#include "SCPCChannel.h"
#include "MissionControlCenter.h"
#include "GEOSatelliteCommunications.h"
#include "GEOSatelliteMobility.h"
#include "GEOSatellite.h"
#include "GEOUtils.h"
#include "Tags.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>

Define_Channel(SCPCChannel);
std::set<double> SCPCChannel::activeCarriers;

using namespace inet;

namespace { // Anonymous namespace to keep these variables file-local
    std::ofstream outputFile; // Single global file stream
    std::mutex fileMutex; // Mutex for file access synchronization
    std::mt19937 rng; // Random number generator
}

SCPCChannel::SCPCChannel() {}  // No implementation here

SCPCChannel::~SCPCChannel() {
    activeCarriers.erase(carrierFrequency);
}

void SCPCChannel::initialize(int stage)
{
    if (stage == 2){
        cDatarateChannel::initialize();

        // Get parameters
        carrierFrequency = par("carrierFrequency").doubleValue();
        bandwidth = par("bandwidth").doubleValue();
        symbolRate = par("symbolRate").doubleValue();
        datarate = par("datarate").doubleValue();
        modulation = par("modulation").stdstringValue();
        weatherModel = par("weatherModel").doubleValue();
        useDynamicWeather = par("useDynamicWeather").boolValue();
        useDynamicWeather = false; // Force always false because handled by MCC

        // Check for carrier frequency collision
        if (activeCarriers.find(carrierFrequency) != activeCarriers.end()) {
            throw cRuntimeError("Carrier frequency %f Hz is already in use", carrierFrequency);
        }

        // Register carrier frequency
        activeCarriers.insert(carrierFrequency);

        // Emit initial signal
        emit(registerSignal("carrierId"), (long)carrierFrequency);

        // Get source and destination modules
        cGate *srcGate = getSourceGate();
        cGate *destGate = srcGate->getNextGate();

        cModule *sourceModule = srcGate->getOwnerModule();
        cModule *destModule = destGate->getOwnerModule();

        // Get antenna modules from source and destination
        auto srcAntenna = check_and_cast<GEOSatelliteAntenna*>(sourceModule->getSubmodule("antenna"));
        auto dstAntenna = check_and_cast<GEOSatelliteAntenna*>(destModule->getSubmodule("antenna"));

        // Check if antenna modules were found
        if (!srcAntenna) {
            throw cRuntimeError("Source antenna module not found");
        }
        if (!dstAntenna) {
            throw cRuntimeError("Destination antenna module not found");
        }

        configName = par("configName").stdstringValue();
        // Open the output file and write headers ONLY ONCE
        { // Scope for the lock
            std::lock_guard<std::mutex> lock(fileMutex); // Acquire lock

            if (!outputFile.is_open()) { // Check if the file is already open
                std::string subfolder = "data"; // Name of the subfolder
                std::string filename = subfolder + "/" + configName + "_scpc_channel_data.txt";
                outputFile.open(filename.c_str());
                if (outputFile.is_open()) {
                    outputFile << "simTime,MCC_idx,atmosphericLoss_dB,fspl_dB,power_dBm,weatherAtMCC" << std::endl;
                } else {
                    throw cRuntimeError("Error opening output file: %s", filename.c_str());
                }
            }
        } // Release lock when leaving scope

        EV << "SCPCChannel initialized with config: " << configName << endl;

        EV << "SCPCChannel initialized: " << endl;
        EV << "    Carrier Frequency: " << carrierFrequency << " Hz" << endl;
        EV << "    Bandwidth: " << bandwidth << " Hz" << endl;
        EV << "    Symbol Rate: " << symbolRate << " Hz" << endl;
        EV << "    Modulation: " << modulation << endl;
        EV << "    Weather Model: " << weatherModel << " is it dynamic? " << useDynamicWeather << endl;
        EV << "    Source Antenna: " << srcAntenna->getInfo() << endl;
        EV << "    Destination Antenna: " << dstAntenna->getInfo() << endl;
    }
}

cChannel::Result SCPCChannel::processMessage(cMessage *msg, const SendOptions& options, simtime_t t)
{
    cChannel::Result result;
    double specWeatherModel = 0.0;
    bool useSpecDynamicWeather = false;
    int specWeatherModelIdx = -1;

    if (msg->isPacket()) {
        auto packet = check_and_cast<Packet*>(msg);
        auto powerTag = packet->addTagIfAbsent<PowerTag>();

        // Get source and destination modules and their positions for FSPL calculation
        cModule *txModule = getSourceGate()->getOwnerModule();
        cModule *rxModule = getSourceGate()->getNextGate()->getOwnerModule(); // Get the next gate's owner

        Coord txPosition, rxPosition;

        if (dynamic_cast<MissionControlCenter*>(txModule)) { // Uplink
            txPosition = check_and_cast<GroundStationMobility*>(txModule->getSubmodule("mobility"))->getRealWorldPosition();
            rxPosition = check_and_cast<GEOSatelliteMobility*>(rxModule->getSubmodule("mobility"))->getRealWorldPosition();
            EV << "UPLINK TRANSMISSION\n";
            specWeatherModel = check_and_cast<MissionControlCenter*>(txModule)->getSpecWeatherModel();
            useSpecDynamicWeather = check_and_cast<MissionControlCenter*>(txModule)->getUseSpecDynamicWeather();
            specWeatherModelIdx = check_and_cast<MissionControlCenter*>(txModule)->getIndex();

        } else if (dynamic_cast<GEOSatellite*>(txModule)) { // Downlink
            txPosition = check_and_cast<GEOSatelliteMobility*>(txModule->getSubmodule("mobility"))->getRealWorldPosition();
            rxPosition = packet->getTag<TargetTag>()->getPosition();
            EV << "DOWNLINK TRANSMISSION\n";
            if (dynamic_cast<MissionControlCenter*>(rxModule)) { // Downlink's MCC
                EV << "DOWNLINK MCC\n";
                specWeatherModel = check_and_cast<MissionControlCenter*>(rxModule)->getSpecWeatherModel();
                useSpecDynamicWeather = check_and_cast<MissionControlCenter*>(rxModule)->getUseSpecDynamicWeather();
                specWeatherModelIdx = check_and_cast<MissionControlCenter*>(rxModule)->getIndex();
            }
        } else {
            throw cRuntimeError("Unsupported module type for SCPCChannel");
        }

        EV << "TX module: " << txModule->getFullName() << " @ " << txPosition << endl;
        EV << "RX module: " << rxModule->getFullName() << " @ " << rxPosition << endl;

        // Step 3: Calculate and apply Free Space Path Loss
        try {
           double fspl_dB = calculateFreeSpacePathLoss(txPosition, rxPosition, carrierFrequency);

           // Weather model usage:
           double atmosphericLoss_dB = 0.0;
           if (useDynamicWeather) {
               EV << "SCPC SPECIFIC useDynamicWeather IS" << useDynamicWeather << " . A LOGICAL ERROR MAY HAVE OCCURED. TERMINATE SIMULATION. TERMINATE SIMULATION. TERMINATE SIMULATION. TERMINATE SIMULATION." << endl;
           } else {
               // THIS IS (always) DONE INSTEAD
               if (useSpecDynamicWeather) {
                   EV << "Specific weather model from sender/receiver MCC " << specWeatherModelIdx << ": " << specWeatherModel << "mm of rain\n";
                   atmosphericLoss_dB = calculateAtmosphericLoss(carrierFrequency, specWeatherModel);
               } else {
                   EV << "SCPC exclusive value for weather model is used: " << weatherModel << "mm of rain\n";
                   atmosphericLoss_dB = calculateAtmosphericLoss(carrierFrequency, weatherModel);
               }
           }

           powerTag->setFSPL_dB(fspl_dB + atmosphericLoss_dB); // Store FSPL in the tag

           EV << "Atmospheric Loss: " << atmosphericLoss_dB << " dB\n";

            double power_dBm;
            if (dynamic_cast<MissionControlCenter*>(txModule)) {
                power_dBm = powerTag->getEIRP_dBm(); // Use EIRP for uplink
            } else {
                power_dBm = powerTag->getTransmitPower_dBm(); //For Downlink we should have transmit power (Pt) by satellite. You'll need to make sure the satellite sets the transmit power in the PowerTag.
            }

            power_dBm -= fspl_dB;
            power_dBm -= atmosphericLoss_dB;
            powerTag->setReceivedPower_dBm(power_dBm);

            { // Scope for the lock
                std::lock_guard<std::mutex> lock(fileMutex);  // Acquire lock

                if (outputFile.is_open()) {
                    EV << "Mutex acquired and is writing to file... " << simTime() << "," << specWeatherModelIdx << "," << atmosphericLoss_dB << "," << fspl_dB << "," << power_dBm << "," << specWeatherModel << endl;
                    outputFile << simTime() << "," << specWeatherModelIdx << "," << atmosphericLoss_dB << "," << fspl_dB << "," << power_dBm << "," << specWeatherModel << std::endl;
                } else {
                    EV_ERROR << "Output file is not open!" << std::endl;
                }

            } // Release lock

            EV << "Step 3 (UPLINK) / 7 (DOWNLINK): FSPL: " << fspl_dB << " dB\n";
            EV << "Step 3 (UPLINK) / 7 (DOWNLINK): Received Power (after FSPL and Atmospheric Loss): " << power_dBm << " dBm\n";

        } catch (const std::exception& e) {
            EV_ERROR << "Error calculating path loss: " << e.what() << endl;
            // Handle the error appropriately, e.g., drop the packet.
            // result.discard = true;  // Example: Discard the packet
            throw; // Re-throw the exception to let higher levels handle it
            return result;
        }

        // Latency Calculation:
        inet::simtime_t propagationDelay = (txPosition.distance(rxPosition)) / 299792458.0; // divided by the speed of light
        inet::simtime_t processingDelay = par("processingDelay").doubleValue(); // Add processing delay parameter to your channel
        inet::simtime_t totalLatency = propagationDelay + processingDelay;

        // Store latency in the packet tag for logging at the receiver
        auto latencyTag = packet->addTagIfAbsent<LatencyTag>(); // Assumes you have a LatencyTag defined
        latencyTag->setLatency(totalLatency);

        EV << "Propagation Delay: " << propagationDelay << " s\n";
        EV << "Processing Delay: " << processingDelay << "s\n";
        EV << "Total Latency: " << totalLatency << " s\n";

        auto tag = packet->addTagIfAbsent<CarrierTag>();
        tag->setCarrierFrequency(carrierFrequency);
    }

    cDatarateChannel::processMessage(msg, options, t);
    return result;
}

void SCPCChannel::finish()
{
    // Clean up carrier registration
    activeCarriers.erase(carrierFrequency);
    cDatarateChannel::finish();

    { // Scope for lock
        std::lock_guard<std::mutex> lock(fileMutex);
        if (outputFile.is_open()) {
            outputFile.close();
        }
    }
}
