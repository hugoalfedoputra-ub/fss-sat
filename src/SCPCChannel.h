#ifndef __SCPCCHANNEL_H__
#define __SCPCCHANNEL_H__

#include <omnetpp.h>
#include <inet/common/TagBase.h>
#include <inet/common/packet/Packet.h>
#include <set>
#include "GEOSatelliteAntenna.h"
#include "Tags.h"
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
    std::string modulation;
    static std::set<double> activeCarriers;
//    GEOSatelliteAntenna *txAntenna;
//    GEOSatelliteAntenna *rxAntenna;

    virtual void initialize(int) override;
    virtual int numInitStages() const override { return 3; }
    virtual cChannel::Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;
    virtual void finish() override;

public:
    SCPCChannel();
    virtual ~SCPCChannel();
//    void setTxAntenna(GEOSatelliteAntenna* antenna) { txAntenna = antenna; }
//    void setRxAntenna(GEOSatelliteAntenna* antenna) { rxAntenna = antenna; }

    double getCarrierFrequency() const { return carrierFrequency; }
    double calculateAtmosphericLoss(double frequencyGHz, double weatherModel) {
        // Simplified ITU-R P.676 model
        return 0.002 * pow(weatherModel, 0.85) * pow(frequencyGHz, 2.3);
    }
};

#endif
