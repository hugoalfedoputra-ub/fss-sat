#include "SCPCChannel.h"
#include <inet/common/ModuleAccess.h>
#include <inet/common/packet/Packet.h>

Define_Channel(SCPCChannel);
std::set<double> SCPCChannel::activeCarriers;


SCPCChannel::SCPCChannel() {}  // No implementation here

SCPCChannel::~SCPCChannel() {
    activeCarriers.erase(carrierFrequency);
}


void SCPCChannel::initialize()
{
    cDatarateChannel::initialize();

    // Get parameters
    carrierFrequency = par("carrierFrequency").doubleValue();
    bandwidth = par("bandwidth").doubleValue();
    symbolRate = par("symbolRate").doubleValue();
    datarate = par("datarate").doubleValue();
    modulation = par("modulation").stdstringValue();

    // Check for carrier frequency collision
    if (activeCarriers.find(carrierFrequency) != activeCarriers.end()) {
        throw cRuntimeError("Carrier frequency %f Hz is already in use", carrierFrequency);
    }

    // Register carrier frequency
    activeCarriers.insert(carrierFrequency);

    // Emit initial signal
    emit(registerSignal("carrierId"), (long)carrierFrequency);

    EV << "SCPCChannel initialized for " << carrierFrequency << endl;
}

cChannel::Result SCPCChannel::processMessage(cMessage *msg, const SendOptions& options, simtime_t t)
{
    cChannel::Result result;
    cDatarateChannel::processMessage(msg, options, t);

    if (msg->isPacket()) {
        inet::Packet *packet = check_and_cast<inet::Packet *>(msg);
        auto tag = packet->addTagIfAbsent<CarrierTag>();
        tag->setCarrierFrequency(carrierFrequency);

    }
    return result;
}

void SCPCChannel::finish()
{
    // Clean up carrier registration
    activeCarriers.erase(carrierFrequency);
    cDatarateChannel::finish();
}
