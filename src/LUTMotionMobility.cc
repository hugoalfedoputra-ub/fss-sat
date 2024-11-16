#include "LUTMotionMobility.h"
#include <cmath>
#include "GEOUtils.h"

using namespace inet;
Define_Module(LUTMotionMobility);

void LUTMotionMobility::initialize(int stage)
{

    StationaryMobility::initialize(stage);
    if (stage == 0) {
        EV << "initializing LUTMotionMobility stage " << stage << endl;
        mapx = std::atoi(getParentModule()->getParentModule()->getDisplayString().getTagArg("bgb", 0));
        mapy = std::atoi(getParentModule()->getParentModule()->getDisplayString().getTagArg("bgb", 1));
        latitude = par("latitude");
        longitude = par("longitude");


        double altitude = 0; // MCCs are at ground level
        realWorldPosition = toECEF(latitude, longitude, altitude);

        setInitialPosition();

        EV << "Latitude: " << latitude << ", Longitude: " << longitude << ", Altitude: " << altitude << endl;
        EV << "ECEF: " << realWorldPosition << endl;
        EV << "LUTMotionMobility stage " << stage << " Initialized" << endl;
    }
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
