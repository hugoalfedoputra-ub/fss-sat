package sat2.simulations;

import inet.mobility.static.StationaryMobility;
import inet.mobility.contract.IMobility;

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
        double cBandDownlinkFrequency @unit(Hz);  // C-band downlink frequency
        double cBandUplinkFrequency @unit(Hz);    // C-band uplink frequency
        @display("i=block/cogwheel");
    gates:
        input in[];  // Changed to vector gate
        output out[];  // Changed to vector gate
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
        @display("i=satellit_blue");
        
    gates:
        input in[];     // Array of input gates from MCCs
        output out[];   // Array of output gates to MCCs
        
    submodules:
        antenna: GEOSatelliteAntenna {
            parameters:
                @display("p=100,50");
        }
        communications: GEOSatelliteCommunications {
            parameters:
                @display("p=200,50");
        }
        mobility: GEOSatelliteMobility {
            parameters:
                @display("p=150,100");
        }
    connections:
        for i=0..sizeof(in)-1 {
            in[i] --> communications.in++;
            communications.out++ --> out[i];
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

    connections allowunconnected:
}

network GroundStations
{
    parameters:
        int numOfMCCs;
        int numOfSatellites;
        @display("bgi=background_earth;bgb=2160,1080");

    types:
        channel SCPCLink extends ned.DatarateChannel
        {
            parameters:
                delay = 250ms;
                datarate = 1Mbps;
        }

    submodules:
        mcc[numOfMCCs]: MissionControlCenter {
            parameters:
                @display("p=240,150;i=misc/building;r=10,,black");
        }
        satellite[numOfSatellites]: GEOSatellite {
            parameters:
                @display("p=150,50");
        }
        
    connections:
        for i=0..numOfMCCs-1 {
            mcc[i].satOut --> SCPCLink --> satellite[0].in++;
            satellite[0].out++ --> SCPCLink --> mcc[i].satIn;
        }
}