#ifndef __GEOSATELLITEANTENNA_H__
#define __GEOSATELLITEANTENNA_H__

#include <omnetpp.h>
#include <inet/common/geometry/common/Coord.h>
#include <inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h>
#include <cmath>

class GEOSatelliteAntenna : public omnetpp::cSimpleModule
{
  protected:
    double diameter;            // in meters
    double beamWidth;          // in degrees
    double gain;               // in dBi
    std::string polarization;  // e.g., "linear", "circular"
    double pointingAccuracy;   // in degrees
    double power;              // in Watts

    // Coverage calculations
    double earthRadius = 6378137.0;  // WGS84 semi-major axis in meters
    double geoRadius = 42164000.0;   // GEO orbit radius in meters

    virtual void initialize() override;
//    virtual double calculateAzimuthAngle(const inet::Coord& targetPosition);
    virtual double calculateFreeSpacePathLoss(const inet::Coord& targetPosition, double frequency); // Added frequency

  public:
    bool isWithinCoverage(const inet::Coord& targetPosition);
    double calculateElevationAngle(const inet::Coord& targetPosition);
};

#endif

 /* GEOSATELLITEANTENNA_H_ */
