#ifndef __GEOSATELLITECOMMUNICATIONS_H__
#define __GEOSATELLITECOMMUNICATIONS_H__

#include <omnetpp.h>
#include <inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h>
#include "SCPCChannel.h"
#include "GEOSatelliteAntenna.h"
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
    physicallayer::IRadio *radio;


    // ... other C-band parameters (bandwidth, power, etc.)

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
 /* GEOSATELLITECOMMUNICATIONS_H_ */
