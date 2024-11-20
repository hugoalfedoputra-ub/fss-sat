#ifndef GEOUTILS_H_
#define GEOUTILS_H_
#include <inet/common/geometry/common/Coord.h>
#include <limits> // Required for numeric_limits
using namespace std;
using namespace inet;

Coord toECEF(double latitude, double longitude, double altitude);
Coord ecefToGeodetic(const Coord& ecef);
double calculateElevationAngle(double satLongitude, double siteLongitude, double siteLatitude) ;
double calculateFreeSpacePathLoss(const Coord& transmitterPosition, const Coord& receiverPosition, double frequency);
double calculateRainLoss(double frequency, double weatherModel, const Coord& txPos, const Coord& rxPos);
double calculateCloudLoss(double frequency, double surfaceHumidity, double elevationAngleDeg, bool partialCloudCover);

#endif /* GEOUTILS_H_ */
