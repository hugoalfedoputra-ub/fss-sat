package sat2.simulations;

import inet.mobility.static.StationaryMobility;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.NetworkInterface;

channel SCPCLink extends ned.DatarateChannel
{
    parameters:
        @class(SCPCChannel);  // Custom channel class for SCPC implementation
        double carrierFrequency @unit(Hz);  // Individual carrier frequency for this link
        double bandwidth @unit(Hz) = default(36MHz);  // Channel bandwidth
        double symbolRate @unit(Hz) = default(27MHz);  // Symbol rate
        string modulation = default("QPSK");  // Modulation scheme
        double bitErrorRate = default(0);  // Bit error rate
        double packetErrorRate = default(0);  // Packet error rate
        @signal[carrierId](type=long);  // Signal for carrier ID tracking
        @statistic[activeCarriers](title="Number of active carriers"; source=carrierId; record=vector,stats);
}

simple LUTMotionMobility extends StationaryMobility
{
    parameters:
        double latitude;
        double longitude;
        initFromDisplayString = false;
        updateFromDisplayString = false;
        @class(LUTMotionMobility);
}

simple GEOSatelliteAntenna
{
    parameters:
        double diameter @unit(m);  // Antenna diameter
        double beamWidth @unit(deg);  // Beam width
        double gain @unit(dB);  // Antenna gain
        string polarization;  // Polarization type
        double pointingAccuracy @unit(deg);  // Pointing accuracy
        double power @unit(W);  // Transmission power
        @display("i=device/antennatower");
}

simple GEOSatelliteCommunications
{
    parameters:
        int numOfMCCs;
        double cBandDownlinkFrequency @unit(Hz);  // C-band downlink frequency
        double cBandUplinkFrequency @unit(Hz);    // C-band uplink frequency
        @display("i=block/cogwheel");
    gates:
        input uplinkIn[numOfMCCs];  // Vector gate for uplink reception
        output broadcastOut[numOfMCCs];  // Single gate that will be replicated to multiple connections
}

simple GEOSatelliteMobility extends StationaryMobility
{
    parameters:
        double longitude;  // Satellite's longitude position
        double altitude @unit(m) = default(35786000m);  // GEO altitude
        @class(GEOSatelliteMobility);
}

module GEOSatellite
{
    parameters:
        int numOfMCCs;
        @display("i=satellit_blue");
        
    gates:
        input uplinkIn[]; 
        output broadcastOut[];   // Changed to vector gate for multiple connections
        
    submodules:
        antenna: GEOSatelliteAntenna {
            parameters:
                @display("p=100,50");
        }
        communications: GEOSatelliteCommunications {
            parameters:
                numOfMCCs = parent.numOfMCCs;
                @display("p=200,50");
        }
        mobility: GEOSatelliteMobility {
            parameters:
                @display("p=150,100");
        }
    connections:
        for i=0..sizeof(uplinkIn)-1 {
            uplinkIn[i] --> communications.uplinkIn[i];
        }
        for i=0..sizeof(broadcastOut)-1 {
            communications.broadcastOut[i] --> broadcastOut[i];
        }
}

module MissionControlCenter
{
    parameters:
        string cityName = default("");
        string mobilityType = default("LUTMotionMobility");
        @display("t=$cityName");
        @node;

    gates:
        input satIn;
        output satOut;

    submodules:
        mobility: LUTMotionMobility {
            parameters:
                @display("p=194,103");
        }
}

network GroundStations
{
    parameters:
        int numOfMCCs;
        int numOfSatellites;
        @display("bgi=background_earth;bgb=2160,1080");

    types:
        channel BroadcastChannel extends ned.IdealChannel
        {
            @class(SatelliteBroadcastChannel);
        }

    submodules:
        mcc[numOfMCCs]: MissionControlCenter {
            parameters:
                @display("p=240,150;i=misc/building;r=10,,black");
        }
        satellite[numOfSatellites]: GEOSatellite {
            parameters:
                numOfMCCs = parent.numOfMCCs;
                @display("p=150,50");
            gates:
                uplinkIn[numOfMCCs]; 
                broadcastOut[numOfMCCs];
        }
        
    connections:
//         Each MCC gets its own unique uplink gate
        for i=0..numOfMCCs-1 {
            mcc[i].satOut --> SCPCLink --> satellite[0].uplinkIn[i];
        }
//         Broadcast connections
        for i=0..numOfMCCs-1 {
            satellite[0].broadcastOut[i] --> BroadcastChannel { @display("ls=red"); } --> mcc[i].satIn;
        }
}