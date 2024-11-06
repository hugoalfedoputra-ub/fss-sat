#include "GEOSatelliteCommunications.h"
#include <inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h>
#include <inet/physicallayer/wireless/common/radio/packetlevel/Radio.h>
#include <inet/physicallayer/wireless/common/contract/packetlevel/IWirelessSignal.h>

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

    int gateIndex = msg->getArrivalGate()->getIndex();

    if (msg->isSelfMessage()) {
        // Handle self messages (e.g., timeouts, periodic checks)
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
            // Signal emission for statistics
            emit(broadcastSignal, packet->getByteLength());

            // Process the packet based on frequency band
            if (radio && radio->getTransmissionState() == physicallayer::IRadio::TRANSMISSION_STATE_IDLE) {
                // Broadcasting logic
                int numGates = gateSize("broadcastOut");
                int successfulTransmissions = 0;

                for (int i = 0; i < numGates; ++i) {
                    if (gate("broadcastOut", i)->isConnected()) {
                        try {
                            Packet *dupPacket = packet->dup();
                            send(dupPacket, "broadcastOut", i);
                            successfulTransmissions++;

                            EV << "Successfully broadcast packet on gate broadcastOut[" << i << "]" << endl;
                        }
                        catch (const cRuntimeError& e) {
                            EV_ERROR << "Failed to broadcast on gate " << i << ": " << e.what() << endl;
                        }
                    }
                }

                EV << "Broadcast completed. Successful transmissions: "
                   << successfulTransmissions << "/" << numGates << endl;
            }
            else {
                EV_WARN << "Radio not idle or not available, packet dropped" << endl;
            }
        }
        catch (const std::exception& e) {
            EV_ERROR << "Error processing packet: " << e.what() << endl;
        }

        // Cleanup original packet
        delete msg;
    }
    else {
        EV_WARN << "Received non-packet message from gate " << gateIndex
                << ", discarding" << endl;
        delete msg;
    }
}
