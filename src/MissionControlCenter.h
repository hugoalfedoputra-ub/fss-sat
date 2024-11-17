#ifndef __MISSIONCONTROLCENTER_H__
#define __MISSIONCONTROLCENTER_H__

#include <omnetpp.h>
#include <inet/common/INETDefs.h>
#include <inet/common/TagBase.h>
#include <inet/common/geometry/common/Coord.h>
#include <random>
#include "GroundStationMobility.h"
#include "GEOSatelliteAntenna.h"

using namespace inet;
using namespace omnetpp;

class MissionControlCenter : public cSimpleModule
{
protected:
    simtime_t iaTime;
    std::mt19937 rng;
    GEOSatelliteAntenna *antenna;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
public:
    MissionControlCenter() {}
};

#endif
