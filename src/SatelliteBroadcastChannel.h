#ifndef __SATELLITEBROADCASTCHANNEL_H__
#define __SATELLITEBROADCASTCHANNEL_H__

#include <omnetpp.h>
#include <inet/common/geometry/common/Coord.h>
#include "SCPCChannel.h"

using namespace omnetpp;

class SatelliteBroadcastChannel : public cChannel
{
  protected:
    virtual void initialize() override;
    virtual Result processMessage(cMessage *msg,
                                const SendOptions& options,
                                simtime_t t) override;

    virtual bool isTransmissionChannel() const override { return true; }

    // Required abstract method implementations
    virtual double getNominalDatarate() const override { return 1e6; } // 1 Mbps default
    virtual bool isBusy() const override { return false; }
    virtual simtime_t getTransmissionFinishTime() const override { return SIMTIME_ZERO; }
    virtual simtime_t calculateDuration(cMessage *msg) const override;

  public:
    SatelliteBroadcastChannel() : cChannel("satelliteBroadcastChannel") {}
};

#endif
