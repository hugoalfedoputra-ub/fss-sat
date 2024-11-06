#ifndef __MISSIONCONTROLCENTER_H__
#define __MISSIONCONTROLCENTER_H__
#include <omnetpp.h>
#include <inet/common/INETDefs.h>
using namespace inet;
using namespace omnetpp;

class MissionControlCenter : public cSimpleModule
{
  protected:
    simtime_t iaTime;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

  public:
     MissionControlCenter() {}
};

#endif
