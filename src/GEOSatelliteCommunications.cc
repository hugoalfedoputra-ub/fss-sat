#include "GEOSatelliteCommunications.h"
using namespace inet;
using namespace omnetpp;

Define_Module(GEOSatelliteCommunications);

void GEOSatelliteCommunications::initialize()
{
    cBandDownlinkFrequency = par("cBandDownlinkFrequency");
    cBandUplinkFrequency = par("cBandUplinkFrequency");
    // ... initialize other parameters
}

void GEOSatelliteCommunications::handleMessage(cMessage *msg)
{
    int gateIndex = msg->getArrivalGate()->getIndex(); // Get the originating MCC's index
    Packet* packet = check_and_cast<Packet*>(msg);

    //Aloha Inspiration: Server in Aloha processes packets and logs events.
    EV << "Satellite received packet from MCC " << gateIndex << endl;

    // Broadcast the packet to all MCCs
    for (int i = 0; i < gateSize("broadcastOut"); ++i) {
        if (gate("broadcastOut", i)->isConnected()) { // Only send to connected gates
            cMessage *copy = packet->dup();
            send(copy, "broadcastOut", i);
        }
    }

//    send(packet->dup(), "broadcastOut", gateIndex); // Send a copy for broadcast
//    delete msg; // Delete the original uplink message
}

void GEOSatelliteCommunications::sendSignal(cMessage *msg, double frequency)
{
    // Implement signal transmission logic here
    EV << "Sending signal on frequency " << frequency/1e9 << " GHz\n";
    send(msg, "out");
}

void GEOSatelliteCommunications::receiveSignal(cMessage *msg)
{
    // Implement signal reception logic here
    EV << "Received signal\n";
}
