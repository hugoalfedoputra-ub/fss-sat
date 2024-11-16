#include "GEOSatelliteMobility.h"
#include <inet/common/INETMath.h>
#include "GEOUtils.h"

using namespace inet;
using namespace inet::math;

Define_Module(GEOSatelliteMobility);

void GEOSatelliteMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);

    if (stage == 0) {
        // Get display dimensions from parent module
        mapx = std::atoi(getParentModule()->getParentModule()->getDisplayString().getTagArg("bgb", 0));
        mapy = std::atoi(getParentModule()->getParentModule()->getDisplayString().getTagArg("bgb", 1));

        longitude = par("longitude");

        realWorldPosition = toECEF(0, longitude, altitude);
        setInitialPosition();
        EV << "GEOSatelliteMobility Initialized" << endl;
    }



}

void GEOSatelliteMobility::setInitialPosition()
{
    // Simplified Earth radius (WGS84 semi-major axis)
    double earthRadius = 6378137.0;

    // Calculate Cartesian coordinates based on longitude and altitude
    double radius = earthRadius + altitude;
    lastPosition.x = ((mapx * longitude) / 360) + (mapx / 2);
    lastPosition.x = static_cast<int>(lastPosition.x) % static_cast<int>(mapx);
    lastPosition.y = ((-mapy * 0) / 180) + (mapy / 2);
    lastPosition = Coord((((mapx * longitude) / 360) + (mapx / 2)), (((-mapy * 0) / 180) + (mapy / 2)), 0);
}

void GEOSatelliteMobility::handleSelfMessage(cMessage *msg)
{
    // For static GEO satellite, we don't need to handle any messages
    // but we must implement this pure virtual method
}
