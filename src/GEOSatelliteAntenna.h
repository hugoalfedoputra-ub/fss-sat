#ifndef __GEOSATELLITEANTENNA_H__
#define __GEOSATELLITEANTENNA_H__

#include <omnetpp.h>
#include <inet/common/geometry/common/Coord.h>
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


  public:
    bool isWithinCoverage(const inet::Coord& targetPosition);
    double calculateElevationAngle(const inet::Coord& targetPosition);
    void setDiameter(double d) { diameter = d; }
    void setBeamWidth(double bw) { beamWidth = bw; }
    void setGain(double g) { gain = g; }
    void setPolarization(const std::string& p) { polarization = p; }
    void setPointingAccuracy(double pa) { pointingAccuracy = pa; }
    void setPower(double p) { power = p; }
    double getPower() const { return power; }
    double getGain() const {return gain;}
    double getDiameter() const {return diameter;}
    virtual std::string getInfo() const {
        std::stringstream out;
        out << "Diameter: " << diameter << " m, ";
        out << "Beam Width: " << beamWidth << " deg, ";
        out << "Gain: " << gain << " dBi, ";
        out << "Polarization: " << polarization << ", ";
        out << "Pointing Accuracy: " << pointingAccuracy << " deg, ";
        out << "Power: " << power << " W";
        return out.str();
    }

};

#endif

 /* GEOSATELLITEANTENNA_H_ */
