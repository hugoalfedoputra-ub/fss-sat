#include "SatelliteBroadcastChannel.h"
#include "GEOSatelliteAntenna.h"
#include "LUTMotionMobility.h"
#include "SCPCChannel.h"

Define_Channel(SatelliteBroadcastChannel);

void SatelliteBroadcastChannel::initialize()
{
    cChannel::initialize();
}

double GEOSatelliteAntenna::calculateAzimuthAngle(const inet::Coord& targetPosition)
{
    // Dummy implementation, replace later
    return 0.0;
}

double GEOSatelliteAntenna::calculateFreeSpacePathLoss(const inet::Coord& targetPosition)
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

cChannel::Result SatelliteBroadcastChannel::processMessage(cMessage *msg,
                                                         const SendOptions& options,
                                                         simtime_t t)
{
    // Get the satellite's antenna module
    cGate *srcGate = msg->getSenderGate();
    cModule *satellite = srcGate->getOwnerModule();
    auto antenna = check_and_cast<GEOSatelliteAntenna*>(
        satellite->getSubmodule("antenna"));

    // Get target ground station position
    cGate *destGate = msg->getArrivalGate();
    if (!destGate) {
        EV << "Message has no destination gate" << endl;
        delete msg;
        Result r;
        r.remainingDuration = SIMTIME_ZERO;
        return r;
    }

    cModule *targetMCC = destGate->getOwnerModule();
    auto mobility = check_and_cast<LUTMotionMobility*>(
        targetMCC->getSubmodule("mobility"));
    inet::Coord targetPos = mobility->getCurrentPosition();

    // Check if target is within coverage
    if (!antenna->isWithinCoverage(targetPos)) {
        // Drop the message if target is outside coverage
        EV << "Target is outside coverage area" << endl;
        delete msg;
        Result r;
        r.remainingDuration = SIMTIME_ZERO;
        r.discard = true;
        return r;
    }

    // Calculate path loss and apply to signal strength
    double pathLoss = antenna->calculateFreeSpacePathLoss(targetPos);
    // TODO: Apply path loss to message parameters

    // Calculate transmission duration
    Result r;
    r.remainingDuration = calculateDuration(msg);
    return r;
}
