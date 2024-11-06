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
    modulation = par("modulation").stdstringValue();

    // Check for carrier frequency collision
    if (activeCarriers.find(carrierFrequency) != activeCarriers.end()) {
        throw cRuntimeError("Carrier frequency %f Hz is already in use", carrierFrequency);
    }

    // Register carrier frequency
    activeCarriers.insert(carrierFrequency);

    // Emit initial signal
    emit(registerSignal("carrierId"), (long)carrierFrequency);
}

cChannel::Result SCPCChannel::processMessage(cMessage *msg, const SendOptions& options, simtime_t t)
{
    cChannel::Result result;
    cDatarateChannel::processMessage(msg, options, t); // Correct order of arguments

    if (msg->isPacket()) {
        inet::Packet *packet = check_and_cast<inet::Packet *>(msg);
        auto tag = packet->addTagIfAbsent<CarrierTag>();
        tag->setCarrierFrequency(carrierFrequency);

         // Apply BER/PER if configured
        if (par("ber").doubleValue() > 0) {
            // Implement bit error simulation
            double ber = par("ber").doubleValue();
            if (dblrand() < ber * packet->getBitLength()) {
                result.discard = true;
            }
        }
    }
    return result; // You MUST return a Result object
}

void SCPCChannel::finish()
{
    // Clean up carrier registration
    activeCarriers.erase(carrierFrequency);
    cDatarateChannel::finish();
}
