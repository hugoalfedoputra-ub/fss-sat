#include "LUTMotionMobility.h"
#include <cmath>

using namespace inet;
Define_Module(LUTMotionMobility);

void LUTMotionMobility::initialize(int stage)
{
    StationaryMobility::initialize(stage);

    EV << "initializing LUTMotionMobility stage " << stage << endl;
    mapx = std::atoi(getParentModule()->getParentModule()->getDisplayString().getTagArg("bgb", 0));
    mapy = std::atoi(getParentModule()->getParentModule()->getDisplayString().getTagArg("bgb", 1));
    latitude = par("latitude");
    longitude = par("longitude");
    setInitialPosition();
}

double LUTMotionMobility::getLUTPositionX() const
{
    return longitude;
}
double LUTMotionMobility::getLUTPositionY() const
{
    return latitude;
}

const Coord& LUTMotionMobility::getCurrentPosition()
{
    //return Coord(longitude, latitude);
    return lastPosition;
}


void LUTMotionMobility::setInitialPosition()
{
   lastPosition.x = ((mapx * longitude) / 360) + (mapx / 2);
   //lastPosition.x = (2160 + (longitude+180))/360;
   lastPosition.x = static_cast<int>(lastPosition.x) % static_cast<int>(mapx);

   lastPosition.y = ((-mapy * latitude) / 180) + (mapy / 2);
   lastPosition = Coord((((mapx * longitude) / 360) + (mapx / 2)), (((-mapy * latitude) / 180) + (mapy / 2)), 0);
   //lastPosition.y = (1080 + (latitude+90))/180;
}
