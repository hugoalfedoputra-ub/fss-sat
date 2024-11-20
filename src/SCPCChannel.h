#ifndef __SCPCCHANNEL_H__
#define __SCPCCHANNEL_H__

#include <omnetpp.h>
#include <inet/common/TagBase.h>
#include <inet/common/packet/Packet.h>
#include <set>
#include "GEOSatelliteAntenna.h"
#include "Tags.h"
#include <fstream>
using namespace omnetpp;

// WeatherModel: THUNDERSTORM, RAIN, SUNNY

class SCPCChannel : public cDatarateChannel
{
protected:
    double carrierFrequency;
    double bandwidth;
    double symbolRate;
    double datarate;
    double weatherModel;
    bool useDynamicWeather;
    std::string modulation;
    static std::set<double> activeCarriers;
    std::string configName;
    virtual void initialize(int) override;
    virtual int numInitStages() const override { return 3; }
    virtual cChannel::Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;
    virtual void finish() override;

public:
    SCPCChannel();
    virtual ~SCPCChannel();
    double getCarrierFrequency() const { return carrierFrequency; }
};

#endif
