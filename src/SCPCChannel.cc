#include <inet/common/ModuleAccess.h>
#include <inet/common/packet/Packet.h>
#include <inet/common/geometry/common/Coord.h>
#include "SCPCChannel.h"
#include "MissionControlCenter.h"
#include "GEOSatelliteCommunications.h"
#include "GEOSatelliteMobility.h"
#include "GEOUtils.h"

Define_Channel(SCPCChannel);
std::set<double> SCPCChannel::activeCarriers;

using namespace inet;

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

        EV << "SCPCChannel initialized: " << endl;
        EV << "    Carrier Frequency: " << carrierFrequency << " Hz" << endl;
        EV << "    Bandwidth: " << bandwidth << " Hz" << endl;
        EV << "    Symbol Rate: " << symbolRate << " Hz" << endl;
        EV << "    Modulation: " << modulation << endl;
        EV << "    Source Antenna: " << srcAntenna->getInfo() << endl;
        EV << "    Destination Antenna: " << dstAntenna->getInfo() << endl;
    }
}

cChannel::Result SCPCChannel::processMessage(cMessage *msg, const SendOptions& options, simtime_t t)
{
    cChannel::Result result;

    if (msg->isPacket()) {
        auto packet = check_and_cast<Packet *>(msg);

        if (!packet->hasTag<TargetTag>()) {  // Check if tag is missing
            throw cRuntimeError("Packet missing TargetPositionTag! Cannot calculate path loss.");
        }

        if (packet->hasTag<TargetTag>()) {
            const Coord& targetPosition = packet->getTag<TargetTag>()->getPosition();

            EV << "FROM " << targetPosition << endl;

            auto mobilityModule = dynamic_cast<GEOSatelliteMobility*>(getSimulation()->getModuleByPath("GroundStations.satellite[0].mobility"));
            if (!mobilityModule) {
                 throw cRuntimeError("Could not find GEOSatelliteMobility module.");
            }
            Coord satPosition = mobilityModule->getRealWorldPosition();

            EV << "Sat Pos : " << satPosition << endl;

            // Calculate Free Space Path Loss using the function from GEOUtils
            try {
                EV << "Calculating FSPL for Freq: " << carrierFrequency << " Hz";
                double pathloss_dB = calculateFreeSpacePathLoss(satPosition, targetPosition, carrierFrequency);
                EV << "Path loss to target: " << pathloss_dB << " dB\n";

                auto powerTag = packet->addTagIfAbsent<PowerTag>();
                double power_dBm = powerTag->getPower_dBm(); // Get initial power (if any) or use a default value if not set.
                power_dBm -= pathloss_dB;                     // Apply path loss
                powerTag->setPower_dBm(power_dBm);            // Update power tag

            } catch (const std::exception& e) {
                EV_ERROR << "Error calculating path loss: " << e.what() << endl;
                // Handle the error appropriately, e.g., drop the packet.
                // result.discard = true;  // Example: Discard the packet
                throw; // Re-throw the exception to let higher levels handle it
               return result;
            }


        } else {
            EV_WARN << "Packet does not have TargetPositionTag, cannot calculate path loss!\n";
        }

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
}
