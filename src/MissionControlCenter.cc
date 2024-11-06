#include "MissionControlCenter.h"
#include <inet/common/packet/Packet.h>
#include <inet/common/packet/chunk/ByteCountChunk.h>
#include <inet/common/Units.h>

using namespace inet;
Define_Module(MissionControlCenter);

void MissionControlCenter::initialize()
{
    iaTime = par("iaTime").doubleValue(); // Get the double value and store it
    scheduleAt(simTime() + iaTime, new cMessage("sendMsg"));
    EV << "MCC iaTime " << iaTime << " Initialized" << endl;
}

void MissionControlCenter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage() && strcmp(msg->getName(), "sendMsg") == 0) {
        auto dataChunk = makeShared<ByteCountChunk>(B(128));
        Packet *packet = new Packet("dataPacket", dataChunk);
        send(packet, "satOut");
        scheduleAt(simTime() + iaTime, new cMessage("sendMsg"));
        delete msg;
    } else if (msg->isPacket()) {
        Packet *receivedPacket = check_and_cast<Packet*>(msg);

        if (msg->getArrivalGateId() == gate("satIn")->getId()) {
            EV << "MCC " << getIndex() << " received DOWNLINK packet: " << receivedPacket << endl;
        } else {
            EV << "MCC " << getIndex() << " received UPLINK packet " << receivedPacket << endl;
        }
    }
}

void MissionControlCenter::finish()
{
    EV << "MCC " << getIndex() << " finished." << endl;
}


