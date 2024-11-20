#ifndef GEOUTILS_H_
#define GEOUTILS_H_
#include <inet/common/geometry/common/Coord.h>
#include <limits> // Required for numeric_limits

inet::Coord toECEF(double latitude, double longitude, double altitude);
double calculateFreeSpacePathLoss(const inet::Coord& transmitterPosition, const inet::Coord& receiverPosition, double frequency);
double calculateAtmosphericLoss(double frequencyGHz, double weatherModel);

#endif /* GEOUTILS_H_ */
