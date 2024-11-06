#include "GEOSatelliteAntenna.h"
#include "GEOSatelliteMobility.h"
#include "SCPCChannel.h"
#include <inet/physicallayer/wireless/common/pathloss/FreeSpacePathLoss.h>
using namespace inet;

Define_Module(GEOSatelliteAntenna);

void GEOSatelliteAntenna::initialize()
{
    diameter = par("diameter");
    beamWidth = par("beamWidth");
    gain = par("gain");
    polarization = par("polarization").stdstringValue();
    pointingAccuracy = par("pointingAccuracy");
    power = par("power");

    EV << "GEOSatelliteAntenna Initialized " << endl;

}

double GEOSatelliteAntenna::calculateFreeSpacePathLoss(const Coord& targetPosition, double frequency) {
    // Get satellite position from mobility module
    auto mobilityModule = check_and_cast<GEOSatelliteMobility*>(
        getParentModule()->getSubmodule("mobility"));
    Coord satPosition = mobilityModule->getCurrentPosition();

    // Calculate distance between satellite and target
    double distance = satPosition.distance(targetPosition);

    // Convert frequency to Hz if given in GHz
    double freq = frequency * 1e9;

    // Speed of light in m/s
    const double c = 3e8;

    // Calculate wavelength
    double wavelength = c / freq;

    // Free Space Path Loss formula: FSPL = (4πd/λ)²
    double fspl = pow((4 * M_PI * distance) / wavelength, 2);

    // Convert to dB
    double fsplDB = 10 * log10(fspl);

    return fsplDB;
}


bool GEOSatelliteAntenna::isWithinCoverage(const Coord& targetPosition)
{
    // Get satellite position from mobility module
    auto mobilityModule = check_and_cast<GEOSatelliteMobility*>(
        getParentModule()->getSubmodule("mobility"));
    Coord satPosition = mobilityModule->getCurrentPosition();

    // Calculate elevation angle to target
    double elevation = calculateElevationAngle(targetPosition);

    // Check if target is within beam width
    // Factor of 0.5 because beamWidth is full angle, we need half-angle
    return elevation >= (90.0 - beamWidth * 0.5);
}

double GEOSatelliteAntenna::calculateElevationAngle(const Coord& targetPosition)
{
    auto mobilityModule = check_and_cast<GEOSatelliteMobility*>(
        getParentModule()->getSubmodule("mobility"));

    // Get display dimensions
    double mapx = getParentModule()->getParentModule()->par("mapx").doubleValue();
    double mapy = getParentModule()->getParentModule()->par("mapy").doubleValue();

    // Convert display coordinates to spherical coordinates
    double targetLat = ((-targetPosition.y + mapy/2) * 180.0) / mapy;
    double targetLon = ((targetPosition.x - mapx/2) * 360.0) / mapx;

    // Convert to radians
    double targetLatRad = targetLat * M_PI / 180.0;
    double targetLonRad = targetLon * M_PI / 180.0;
    double satLonRad = mobilityModule->getLongitude() * M_PI / 180.0;

    // Calculate central angle between satellite and target
    double centralAngle = acos(cos(targetLatRad) * cos(targetLonRad - satLonRad));

    // Calculate elevation angle using satellite altitude
    double satAlt = mobilityModule->getAltitude();
    double elevation = atan2(cos(centralAngle) - earthRadius/(earthRadius + satAlt),
                           sin(centralAngle)) * 180.0 / M_PI;

    return elevation;
}
