#ifndef __SCPCCHANNEL_H__
#define __SCPCCHANNEL_H__

#include <omnetpp.h>
#include <inet/common/TagBase.h>
#include <inet/common/packet/Packet.h>
#include <set>

using namespace omnetpp;

class CarrierTag : public inet::TagBase
{
private:
    double carrierFrequency;
public:
    CarrierTag() : carrierFrequency(0) {}
    virtual void setCarrierFrequency(double freq) { carrierFrequency = freq; }
    virtual double getCarrierFrequency() const { return carrierFrequency; }
    virtual const char* getName() const override { return "CarrierTag"; }
    virtual std::string str() const override { return ""; }
};

class SCPCChannel : public cDatarateChannel
{
protected:
    double carrierFrequency;
    double bandwidth;
    double symbolRate;
    double datarate;
    std::string modulation;
    static std::set<double> activeCarriers;

    virtual void initialize() override;
    virtual cChannel::Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;
    virtual void finish() override;

public:
    SCPCChannel();
    virtual ~SCPCChannel();

};

#endif
