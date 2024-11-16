#ifndef GEOUTILS_H_
#define GEOUTILS_H_
#include <inet/common/geometry/common/Coord.h>

inet::Coord toECEF(double latitude, double longitude, double altitude);
double calculateFreeSpacePathLoss(const inet::Coord& transmitterPosition, const inet::Coord& receiverPosition, double frequency); // New function declaration

#endif /* GEOUTILS_H_ */
