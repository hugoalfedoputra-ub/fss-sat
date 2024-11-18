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
    std::vector<double> weatherModels; // Vector to store weather model for each MCC
//    std::ofstream outputFile;
//    GEOSatelliteAntenna *txAntenna;
//    GEOSatelliteAntenna *rxAntenna;
    std::string configName;
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
        return 0.002 * pow(weatherModel, 0.85) * pow((frequencyGHz / 1000000000.0), 2.3);
    }

    // Will be deprecated
    double calculateAtmosphericLoss(double frequencyGHz, int targetMCC) {
        // Simplified ITU-R P.676 model
        double weatherModel = weatherModels[targetMCC];
        return 0.002 * pow(weatherModel, 0.85) * pow((frequencyGHz / 1000000000.0), 2.3);
    }
};

#endif
