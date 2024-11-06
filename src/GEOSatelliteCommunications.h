#ifndef __GEOSATELLITECOMMUNICATIONS_H__
#define __GEOSATELLITECOMMUNICATIONS_H__

#include <omnetpp.h>
#include "SCPCChannel.h"

class GEOSatelliteCommunications : public omnetpp::cSimpleModule
{
  protected:
    // C-band properties
    double cBandDownlinkFrequency; // in Hz
    double cBandUplinkFrequency;   // in Hz
    // ... other C-band parameters (bandwidth, power, etc.)

    // SCPC parameters
    struct SCPCLink {
        int linkId;
        double carrierFrequency;
        double bandwidth;
        double symbolRate;
        std::string modulation;
        bool active;
    };

    std::map<int, SCPCLink> scpcLinks;  // Maps linkId to SCPC link parameters

    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

  public:
    // Example methods for sending and receiving
    void sendSignal(omnetpp::cMessage *msg, double frequency);
    void receiveSignal(omnetpp::cMessage *msg);
};

#endif
 /* GEOSATELLITECOMMUNICATIONS_H_ */
