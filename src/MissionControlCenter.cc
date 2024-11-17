#include "MissionControlCenter.h"
#include <inet/common/packet/Packet.h>
#include <inet/common/packet/chunk/ByteCountChunk.h>
#include <inet/common/Units.h>
#include <inet/common/INETMath.h>
#include "Tags.h"
#include <random>

using namespace inet;
using namespace inet::math;
Define_Module(MissionControlCenter);

void MissionControlCenter::initialize(int stage)
{
//    EV_INFO << "GEOSatellite::initialize stage " << stage << endl;
    if (stage == 1) {
        iaTime = par("iaTime").doubleValue();
        scheduleAt(simTime() + iaTime, new cMessage("sendMsg"));

        // Initialize random number generator
        rng.seed(time(0) + getIndex()); // Seed based on time and MCC index

        antenna = check_and_cast<GEOSatelliteAntenna*>(getSubmodule("antenna"));
        if (!antenna) {
            throw cRuntimeError("Antenna module not found in MCC");
        }

        EV << "MCC " << getIndex() << " iaTime " << iaTime << " Initialized" << endl;
        EV << antenna->getInfo() << endl;
    }
}

void MissionControlCenter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage() && strcmp(msg->getName(), "sendMsg") == 0) {
        auto dataChunk = makeShared<ByteCountChunk>(B(128));
        Packet *packet = new Packet("dataPacket", dataChunk);

        // Determine target MCC (randomly, but not self)
        int targetMCC = getIndex();
        while (targetMCC == getIndex()) {
            targetMCC = std::uniform_int_distribution<int>(0, getParentModule()->par("numOfMCCs").intValue() - 1)(rng);
        }

        // Add target MCC as a tag to the packet
        auto targetTag = packet->addTagIfAbsent<TargetTag>();
        targetTag->setTarget(targetMCC);

        auto mobility = getSubmodule("mobility");
        if (mobility) {
            auto groundStationMobility = dynamic_cast<GroundStationMobility*>(mobility);
            if (groundStationMobility) {
                targetTag->setPosition(groundStationMobility->getRealWorldPosition());
            } else {
                EV_ERROR << "Mobility submodule is not of type LUTMotionMobility" << endl;
            }
        } else {
            EV_ERROR << "Mobility submodule not found" << endl;
        }

        auto powerTag = packet->addTagIfAbsent<PowerTag>();
        powerTag->setPower_mW(antenna->getPower());

        EV << "MCC " << getIndex() << " sending packet to MCC " << targetMCC << endl;

//        simtime_t randomDelay = uniform(1.0, 2.0); // Example: up to 1ms delay
//        scheduleAt(simTime() + randomDelay, packet); // Schedule the packet with a small delay

        send(packet, "satOut");

        scheduleAt(simTime() + iaTime, new cMessage("sendMsg"));
        delete msg;
    } else if (msg->isPacket()) {
        Packet *receivedPacket = check_and_cast<Packet*>(msg);

        if (msg->getArrivalGateId() == gate("satIn")->getId()) {
            EV << "MCC " << getIndex() << " received DOWNLINK packet: " << receivedPacket << " from MCC " << receivedPacket->getTag<TargetTag>()->getTarget() <<  endl;
        } else {
            EV << "MCC " << getIndex() << " received UPLINK packet " << receivedPacket << endl;
        }
    }
}
void MissionControlCenter::finish()
{
    EV << "MCC " << getIndex() << " finished." << endl;
}


