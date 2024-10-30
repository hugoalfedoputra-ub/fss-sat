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
class LUTMotionMobility : public inet::StationaryMobility
{
public:
    // returns longitude
    double getLUTPositionX() const;

    // returns latitude
    double getLUTPositionY() const;

    virtual const Coord& getCurrentPosition() override;

protected:
    virtual void initialize(int) override;

    virtual void setInitialPosition() override;

    double latitude, longitude;   // Geographic coordinates
    double mapx, mapy;            // Coordinates on map
};

#endif
