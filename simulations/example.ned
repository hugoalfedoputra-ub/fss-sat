//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

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

// Bundles the analyzation and notification modules for the OS³ missions.
//
module MissionControlCenter
{
    parameters:
        string cityName = default("");
        string mobilityType = default("LUTMotionMobility"); // Define mobility module
        @display("t=$cityName");
        @node;

//    gates:
//        input radioIn; // Input gate
//        output radioOut; // Output gate

    submodules:
        mobility: LUTMotionMobility {
            parameters:
                @display("p=194,103");
        }
    connections allowunconnected: // Connections between compound gates and submodules

}

network GroundStations
{
    parameters:
        int numOfMCCs;
        @display("bgi=background_earth;bgb=2160,1080");

    submodules:
        mcc[numOfMCCs]: MissionControlCenter {
            parameters:
                @display("p=240,150;i=misc/building;r=10,,black");
        }
    connections allowunconnected:
}
