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
    GEOSatelliteAntenna *antenna; // Pointer to the antenna module
    simsignal_t broadcastSignal;
    std::queue<Packet*> packetQueue; // Queue for storing packets
    cMessage *queueProcessingEvent;

    // ... other C-band parameters (bandwidth, power, etc.)

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void processQueue();
};

#endif
