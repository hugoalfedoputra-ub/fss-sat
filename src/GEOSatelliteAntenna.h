#ifndef __GEOSATELLITEANTENNA_H__
#define __GEOSATELLITEANTENNA_H__

#include <omnetpp.h>

class GEOSatelliteAntenna : public omnetpp::cSimpleModule
{
  protected:
    // Antenna parameters
    double diameter;            // in meters
    double beamWidth;          // in degrees
    double gain;               // in dBi
    std::string polarization;  // e.g., "linear", "circular"
    double pointingAccuracy;    // in degrees
    double power;              // in Watts


    virtual void initialize() override;

  public:
    // Methods to calculate coverage footprint, etc.
    // Example:
//    bool isWithinCoverage(const inet::Coord& targetPosition);

};

#endif /* GEOSATELLITEANTENNA_H_ */
