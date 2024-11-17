#ifndef __GEOSATELLITECOMMUNICATIONS_H__
#define __GEOSATELLITECOMMUNICATIONS_H__

#include <omnetpp.h>
#include "SCPCChannel.h"
#include "GEOSatelliteAntenna.h"
#include <queue>

using namespace omnetpp;
using namespace inet;

class GEOSatelliteCommunications : public cSimpleModule
{
  protected:
    // C-band properties
    double cBandDownlinkFrequency; // in Hz
    double cBandUplinkFrequency;   // in Hz
    double noiseFloor_dBm;
    simsignal_t broadcastSignal;
    std::queue<Packet*> packetQueue; // Queue for storing packets
    cMessage *queueProcessingEvent;

    std::string configName;
    // ... other C-band parameters (bandwidth, power, etc.)

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void processQueue();
    virtual void finish() override;

  public:
    virtual std::string getInfo() const {
        std::stringstream out;
        out << "C-Band Downlink Frequency: " << cBandDownlinkFrequency << " Hz, ";
        out << "C-Band Uplink Frequency: " << cBandUplinkFrequency << " Hz, ";
        out << "Queue Size: " << packetQueue.size();
        return out.str();
    }
};


#endif
