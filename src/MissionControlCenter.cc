#include "MissionControlCenter.h"
#include <inet/common/packet/Packet.h>
#include <inet/common/packet/chunk/ByteCountChunk.h> // For creating data chunks
#include <inet/common/Units.h> // Include for units (B)

using namespace inet;
Define_Module(MissionControlCenter);

void MissionControlCenter::initialize()
{
    iaTime = &par("iaTime");

    EV << "initializing MCC iaTime " << iaTime << endl;

    scheduleAt(simTime() + iaTime->doubleValue(), new cMessage("sendMsg"));

}

void MissionControlCenter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage() && strcmp(msg->getName(), "sendMsg") == 0) {
        // Create a data chunk
        auto dataChunk = makeShared<ByteCountChunk>(B(128));

        // Create a new packet
        Packet *packet = new Packet("dataPacket", dataChunk);

        send(packet, "satOut");

        scheduleAt(simTime() + iaTime->doubleValue(), msg); // Schedule next send

    } else if (msg->isPacket()) { // Handle downlink messages
        Packet *receivedPacket = check_and_cast<Packet*>(msg);

        // ... process receivedPacket (e.g., extract data, log reception)
        EV << "MCC " << getIndex() << " received downlink packet: " << receivedPacket << endl;
//        delete msg;
    }
}
