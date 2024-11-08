#include "GEOSatelliteCommunications.h"
#include <inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h>
#include <inet/physicallayer/wireless/common/radio/packetlevel/Radio.h>
#include <inet/physicallayer/wireless/common/contract/packetlevel/IWirelessSignal.h>
#include "MissionControlCenter.h"

using namespace inet;
using namespace omnetpp;

Define_Module(GEOSatelliteCommunications);

void GEOSatelliteCommunications::initialize()
{
    cBandDownlinkFrequency = par("cBandDownlinkFrequency");
    cBandUplinkFrequency = par("cBandUplinkFrequency");

    antenna = check_and_cast<GEOSatelliteAntenna*>(getParentModule()->getSubmodule("antenna"));

    // Register the broadcast signal
    broadcastSignal = registerSignal("broadcastPackets");

    // ... initialize other parameters
    EV << "GEOSatelliteCommunications Initialized " << endl;

}

void GEOSatelliteCommunications::handleMessage(cMessage *msg)
{
    // Check if the message is null
        if (!msg) {
            EV_ERROR << "Received null message" << endl;
            return;
        }


        if (msg->isSelfMessage()) {
            // Handle self messages (if any)
            EV << "Handling self message: " << msg->getName() << endl;
            delete msg;
            return;
        }

        if (msg->isPacket()) {
            Packet* packet = check_and_cast<Packet*>(msg);

            // Log received packet details
            EV << "Satellite received packet: " << packet->getName()
               << " from gate: " << msg->getArrivalGate()->getFullName()
               << " size: " << packet->getByteLength() << " bytes" << endl;

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
                   EV_WARN << "No output gate found for target MCC " << targetMCC << ", dropping packet" << endl; // Corrected line
                   delete packet;
               }

            } catch (const std::exception& e) {
                EV_ERROR << "Error processing packet: " << e.what() << endl;
                delete msg; // Delete the message in case of error
            }


        } else {
            EV_WARN << "Received non-packet message, discarding" << endl;
            delete msg;
        }
}
