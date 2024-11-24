package sat2.simulations;

import inet.mobility.static.StationaryMobility;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.NetworkInterface;

simple GroundStationMobility extends StationaryMobility
{
    parameters:
        double latitude;
        double longitude;
        initFromDisplayString = false;
        updateFromDisplayString = false;
        @class(GroundStationMobility);
}

simple GEOSatelliteAntenna
{
    parameters:
        double diameter @unit(m);
        double beamWidth @unit(deg);
        double gain @unit(dB);
        string polarization;
        double pointingAccuracy @unit(deg);
        double power @unit(W);
        @display("i=device/antennatower");
}

simple GEOSatelliteCommunications
{
    parameters:
        string configName;
        // Communication parameters
        int numOfMCCs;
        double cBandDownlinkFrequency @unit(Hz);
        double cBandUplinkFrequency @unit(Hz);
		double noiseFloor @unit(dBm);
        @display("i=block/cogwheel");
    gates:
        input uplinkIn[numOfMCCs];
        output downlinkOut[numOfMCCs];
}

simple GEOSatelliteMobility extends StationaryMobility
{
    parameters:
        double longitude;
        double altitude @unit(m) = default(35786000m);
        @class(GEOSatelliteMobility);
}

module GEOSatellite
{
    parameters:
        int numOfMCCs;
        @display("i=satellit_blue");
        @class(GEOSatellite);
        
    gates:
        input uplinkIn[numOfMCCs]; 
        output downlinkOut[numOfMCCs];
        
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
        for i=0..numOfMCCs-1 {
            uplinkIn[i] --> communications.uplinkIn[i];
            communications.downlinkOut[i] --> downlinkOut[i];
        }
}

module MissionControlCenter
{
    parameters:
        string configName;
        string cityName = default("");
        string mobilityType = default("GroundStationMobility");
        double iaTime @unit(s);
        bool useSpecDynamicWeather = default(true);
		bool useDynamicCloudCover = default(true);
		bool useDynamicCloudHeight = default(true);
        double specWeatherModel = default(-100.0); // temp val so noticable in logs
        @class(MissionControlCenter);
        @display("t=$cityName");
        double noiseFloor @unit(dBm); 
    gates:
        input satIn;
        output satOut;

    submodules:
        mobility: GroundStationMobility {
            parameters:
                @display("p=194,103");
        }
        antenna: GEOSatelliteAntenna {
            parameters:
                @display("p=150,150");
        }
        
}

network GroundStations
{
    parameters:
        int numOfMCCs;
        int numOfSatellites;
        @display("bgi=background_earth;bgb=2160,1080");

    submodules:
        mcc[numOfMCCs]: MissionControlCenter {
            parameters:
                @display("p=240,150;i=misc/building;r=10,,black");
        }
        satellite[numOfSatellites]: GEOSatellite {
            parameters:
                numOfMCCs = parent.numOfMCCs;
                @display("p=150,50");
        }
        
    connections:
        for i=0..numOfMCCs-1, for j=0..numOfSatellites-1 {
            mcc[i].satOut --> SCPCLink --> satellite[j].uplinkIn[i];
            satellite[j].downlinkOut[i] --> SCPCLink --> mcc[i].satIn;
        }
}

channel SCPCLink extends ned.DatarateChannel
{
    parameters:
        string configName;
        @class(SCPCChannel);
        double carrierFrequency @unit(Hz);
        double bandwidth @unit(Hz) = default(36MHz);
        double symbolRate @unit(Hz) = default(27MHz);
        string modulation = default("QPSK");
        datarate = default(100Mbps);
        double weatherModel @unit(mm); 		// May be deprecated
        bool useDynamicWeather; 			// May be deprecated; if true then do nothing because handled by MCC, if false then keep current implementaiton
        double processingDelay = default(0.01); // 10 us
        double bitErrorRate = default(0);
        double packetErrorRate = default(0);
        @signal[carrierId](type=long);
        @statistic[activeCarriers](title="Number of active carriers"; source=carrierId; record=vector,stats);
}