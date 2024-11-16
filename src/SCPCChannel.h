#ifndef __SCPCCHANNEL_H__
#define __SCPCCHANNEL_H__

#include <omnetpp.h>
#include <inet/common/TagBase.h>
#include <inet/common/packet/Packet.h>
#include <set>
#include "GEOSatelliteAntenna.h"
#include "Tags.h"

using namespace omnetpp;

class SCPCChannel : public cDatarateChannel
{
protected:
    double carrierFrequency;
    double bandwidth;
    double symbolRate;
    double datarate;
    std::string modulation;
    static std::set<double> activeCarriers;
    GEOSatelliteAntenna antenna;

    virtual void initialize() override;
    virtual cChannel::Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;
    virtual void finish() override;

public:
    SCPCChannel();
    virtual ~SCPCChannel();
    double getCarrierFrequency() const { return carrierFrequency; }
};

#endif
