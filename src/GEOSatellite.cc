#include "GEOSatellite.h"

using namespace omnetpp;

Define_Module(GEOSatellite);
void GEOSatellite::initialize(int stage)
{
//    EV_INFO << "GEOSatellite::initialize stage " << stage << endl;

    if (stage == 1) {
        cModule *antennaModule = getSubmodule("antenna");
        cModule *commsModule = getSubmodule("communications");
        cModule *mobilityModule = getSubmodule("mobility");

        if (!antennaModule || !commsModule || !mobilityModule) {
            throw cRuntimeError("Required submodules not found in GEOSatellite");
        }

        // Now perform the specific casts
        antenna = check_and_cast<GEOSatelliteAntenna *>(antennaModule);
        communications = check_and_cast<GEOSatelliteCommunications *>(commsModule);
        mobility = check_and_cast<GEOSatelliteMobility *>(mobilityModule);

        EV << "GEOSatellite " << endl;
        EV << antenna->getInfo() << endl;
        EV << communications->getInfo() << endl;
        EV << mobility->getInfo() << endl;
    }
}

void GEOSatellite::handleMessage(cMessage *msg)
{
    // If you need the satellite module to handle messages directly, implement the logic here.
    // Otherwise, you can remove this function entirely. Submodules will handle their own messages.
}
