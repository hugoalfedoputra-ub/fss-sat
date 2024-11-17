#ifndef __OS3_LUTMotionMobility_H__
#define __OS3_LUTMotionMobility_H__

#include <omnetpp.h>

#include <inet/mobility/static/StationaryMobility.h>    // inet

using namespace inet;
//-----------------------------------------------------
// Class: LUTMotionMobility
//
// Positions a ground station at a specific lat/long
//-----------------------------------------------------
class GroundStationMobility : public inet::StationaryMobility
{
public:
    // returns longitude
    double getLUTPositionX() const;

    // returns latitude
    double getLUTPositionY() const;
    const Coord& getRealWorldPosition() const { return realWorldPosition; }

    virtual const Coord& getCurrentPosition() override;

protected:
    virtual void initialize(int) override;

    virtual void setInitialPosition() override;
    Coord realWorldPosition;
    double latitude, longitude;   // Geographic coordinates
    double mapx, mapy;            // Coordinates on map
};

#endif
