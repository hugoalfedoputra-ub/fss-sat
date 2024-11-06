#ifndef __MISSIONCONTROLCENTER_H__
#define __MISSIONCONTROLCENTER_H__

#include <omnetpp.h>
#include <inet/common/INETDefs.h>

class MissionControlCenter : public omnetpp::cSimpleModule
{
  protected:
    inet::cPar *iaTime;
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

  public:
     MissionControlCenter() {}
     virtual ~MissionControlCenter() {}
};

#endif
