#include "GEOUtils.h"
#include <inet/common/INETDefs.h>
#include <inet/common/INETMath.h>

using namespace std;
using namespace inet;
using namespace inet::math;

double calculateFreeSpacePathLoss(const Coord& transmitterPosition, const Coord& receiverPosition, double frequency) {
    try {
        // Input validation
        if (frequency <= 0) {
            throw invalid_argument("Frequency must be positive");
        }

        // Calculate distance
        double distance = transmitterPosition.distance(receiverPosition);
        if (distance <= 0) {
            throw domain_error("Distance between transmitter and receiver must be positive.");
        }

        double freq = frequency;

        // Speed of light in m/s
        const double c = 299792458.0;

        // Calculate wavelength
        double wavelength = c / freq;

        // Free Space Path Loss formula: FSPL = 20 * log_10 ( (4*pi*d) / wavelen )
        double fspl = (4 * M_PI * distance) / wavelength;

         // Check for potential numerical overflow
        if (isinf(fspl) || std::isnan(fspl)) {
            throw overflow_error("Numerical overflow in FSPL calculation");
        }

        // Convert to dB
        double fsplDB = 20 * log10(fspl);
        if (isinf(fsplDB) || std::isnan(fsplDB)) {
            throw overflow_error("Numerical overflow in dB conversion");
        }

        return fsplDB;
    }
    catch (const exception& e) {
        EV_ERROR << "Error in Free Space Path Loss Calculation: " << e.what() << "\n";
        throw;
    }
}

Coord toECEF(double latitude, double longitude, double altitude) {
    double a = 6378137.0; // WGS84 semi-major axis
    double f = 1.0 / 298.257223563; // WGS84 flattening
    double b = a * (1.0 - f); // WGS84 semi-minor axis
    double N = a / sqrt(1.0 - f * sin(deg2rad(latitude)) * sin(deg2rad(latitude)));

    double x = (N + altitude) * cos(deg2rad(latitude)) * cos(deg2rad(longitude));
    double y = (N + altitude) * cos(deg2rad(latitude)) * sin(deg2rad(longitude));
    double z = (b * b / (a * a) * N + altitude) * sin(deg2rad(latitude));

    return Coord(x, y, z);
}
