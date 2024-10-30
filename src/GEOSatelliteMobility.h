#ifndef __GEOSATELLITEMOBILITY_H__
#define __GEOSATELLITEMOBILITY_H__

#include <omnetpp.h>
#include <inet/mobility/base/MobilityBase.h>
#include <inet/common/geometry/common/Quaternion.h>

using namespace inet;

class GEOSatelliteMobility : public MobilityBase
{
  protected:
    double longitude;
    double altitude = 35786000; // GEO altitude in meters
    double mapx, mapy;          // Display dimensions
    Coord initialPosition;
    Quaternion zeroQuaternion;

    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void handleSelfMessage(cMessage *msg) override;

  public:
    GEOSatelliteMobility() : zeroQuaternion(0, 0, 0, 0) {}

    virtual const Coord& getCurrentPosition() override { return lastPosition; }
    virtual const Coord& getCurrentVelocity() override { return Coord::ZERO; }
    virtual const Coord& getCurrentAcceleration() override { return Coord::ZERO; }
    virtual const Quaternion& getCurrentAngularPosition() override { return Quaternion::IDENTITY; }
    virtual const Quaternion& getCurrentAngularVelocity() override { return zeroQuaternion; }
    virtual const Quaternion& getCurrentAngularAcceleration() override { return zeroQuaternion; }
    virtual double getMaxSpeed() const override { return 0; }
};

#endif /* GEOSATELLITEMOBILITY_H_ */