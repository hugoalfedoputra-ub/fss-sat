#include "SatelliteBroadcastChannel.h"
#include "GEOSatelliteAntenna.h"
#include "LUTMotionMobility.h"
#include "SCPCChannel.h"
using namespace inet;

Define_Channel(SatelliteBroadcastChannel);

void SatelliteBroadcastChannel::initialize()
{
    cChannel::initialize();
}

double GEOSatelliteAntenna::calculateAzimuthAngle(const Coord& targetPosition)
{
    // Dummy implementation, replace later
    return 0.0;
}

double GEOSatelliteAntenna::calculateFreeSpacePathLoss(const Coord& targetPosition)
{
    // Dummy implementation for now, replace with actual calculation later
    return 0.0;
}

simtime_t SatelliteBroadcastChannel::calculateDuration(cMessage *msg) const
{
    // Assuming a fixed packet size of 1000 bits if not specified
    int bits = 1000;
    if (msg->isPacket())
        bits = check_and_cast<cPacket *>(msg)->getBitLength();

    return simtime_t(bits / getNominalDatarate());
}

cChannel::Result SatelliteBroadcastChannel::processMessage(cMessage *msg, const SendOptions& options, simtime_t t)
{
    // Get the satellite's antenna module (for future use, if needed)
    cGate *srcGate = msg->getSenderGate();
    cModule *satellite = srcGate->getOwnerModule();
    auto *antenna = check_and_cast<GEOSatelliteAntenna*>(satellite->getSubmodule("antenna"));

    // Iterate through all connected output gates (MCCs)
    int numMCCs = satellite->gateSize("broadcastOut");
    for (int i = 0; i < numMCCs; ++i) {
        cGate *destGate = satellite->gate("broadcastOut", i);
        if (destGate->isConnected()) {  // Check if the gate is actually connected
            cModule *targetMCC = destGate->getPathEndGate()->getOwnerModule();
            auto *mobility = check_and_cast<LUTMotionMobility*>(targetMCC->getSubmodule("mobility"));
            Coord targetPos = mobility->getCurrentPosition();

            cPacket *copy = check_and_cast<cPacket*>(msg->dup());

            // Pass along the SendOptions we received
            destGate->deliver(copy, options, t);
            EV << "Broadcasting message to MCC " << i << endl;
        }
    }

    Result r;
    r.remainingDuration = SIMTIME_ZERO;
    return r;
}

