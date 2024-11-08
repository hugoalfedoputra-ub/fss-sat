#ifndef __MISSIONCONTROLCENTER_H__
#define __MISSIONCONTROLCENTER_H__

#include <omnetpp.h>
#include <inet/common/INETDefs.h>
#include <inet/common/TagBase.h>
#include <random>

using namespace inet;
using namespace omnetpp;

class TargetTag : public inet::TagBase
{
private:
    int targetMCC;
public:
    TargetTag() : targetMCC(-1) {}
    virtual void setTarget(int target) { targetMCC = target; }
    virtual int getTarget() const { return targetMCC; }
    virtual const char* getName() const override { return "TargetTag"; }
    virtual std::string str() const override { return ""; }
};


class MissionControlCenter : public cSimpleModule
{
protected:
    simtime_t iaTime;
    std::mt19937 rng; // Random number generator

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
public:
    MissionControlCenter() {}
};

#endif
