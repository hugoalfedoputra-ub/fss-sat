#include "GEOUtils.h"
#include <inet/common/INETDefs.h>
#include <inet/common/INETMath.h>

using namespace std;
using namespace inet;
using namespace inet::math;

// Helper function to convert ECEF to geodetic coordinates
Coord ecefToGeodetic(const Coord& ecef) {
    double a = 6378137.0; // WGS84 semi-major axis
    double f = 1.0 / 298.257223563; // WGS84 flattening
    double b = a * (1.0 - f); // WGS84 semi-minor axis

    double x = ecef.x;
    double y = ecef.y;
    double z = ecef.z;

    double r = sqrt(x * x + y * y);
    double lat = atan2(z, r * (1 - f));
    double lon = atan2(y, x);
    double alt = 0.0;  // Initialize altitude
    double N = 0.0;

    for (int i = 0; i < 5; ++i) { // Iterate for better accuracy
        N = a / sqrt(1 - f * (2 - f) * sin(lat) * sin(lat));
        alt = r / cos(lat) - N;
        lat = atan2(z, r * (1 - f * N / (N + alt)));
    }

    return Coord(rad2deg(lat), rad2deg(lon), alt);
}

// site is MCC
double calculateElevationAngle(double satLongitude, double siteLongitude, double siteLatitude) {
    double G = deg2rad(satLongitude - siteLongitude);   // Longitude difference in radians
    double L = deg2rad(siteLatitude);                   // Site latitude in radians

    double numerator = cos(G) * cos(L) - 0.1512;
    double denominator = sqrt(1.0 - pow(cos(G) * cos(L), 2.0));

    if (denominator == 0.0) {
        // Handle the case where the denominator is zero (e.g., return a default value or throw an exception)
        EV_WARN << "Denominator is zero in elevation angle calculation. Returning 0.\n";
        return 0.0; // Or throw an exception
    }

    double elevationRad = atan(numerator / denominator);
    double elevationDeg = rad2deg(elevationRad);

    return elevationDeg;
}


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

double calculateRainLoss(double frequency, double weatherModel, const Coord& txPos, const Coord& rxPos) {
    // ITU-R P.838-3 model
    double freq = frequency / 1e9; // Need GHz
    double rainLoss_dB = 0.0;
    double k = 0.0;
    double alpha = 0.0;
    double kh = 0.0;
    double kv = 0.0;
    double alphah = 0.0;
    double alphav = 0.0;
    double tau = M_PI / 4.0;

    Coord txGeo = ecefToGeodetic(txPos);
    Coord rxGeo = ecefToGeodetic(rxPos);

    double mccLatitude = txGeo.x;
    double mccLongitude = txGeo.y;
    double satLatitude = rxGeo.x;
    double satLongitude = rxGeo.y;

    double elevationAngle = calculateElevationAngle(satLongitude, mccLongitude, mccLatitude);

    if (4.0 <= freq && freq < 4.5) {
        kh = 0.0001071;
        kv = 0.0002461;
        alphah = 1.6009;
        alphav = 1.2476;
    } else if (6.0 <= freq && freq < 7.0) {
        kh = 0.0007056;
        kv = 0.0004878;
        alphah = 1.5900;
        alphav = 1.5728;
    } else if (12.0 <= freq && freq < 13.0) {
        kh = 0.02386;
        kv = 0.02455;
        alphah = 1.1825;
        alphav = 1.1216;
    } else if (14.0 <= freq && freq < 15.0) {
        kh = 0.03738;
        kv = 0.04126;
        alphah = 1.1396;
        alphav = 1.0646;
    } else if (20.0 <= freq && freq < 21.0) {
        kh = 0.09164;
        kv = 0.09611;
        alphah = 1.0568;
        alphav = 0.9847;
    } else if (30.0 <= freq && freq < 31.0) {
        kh = 0.2403;
        kv = 0.9485;
        alphah = 0.2291;
        alphav = 0.9129;
    } else {
        EV << "FREQUENCY IS OUT OF IMPLEMENTATION FOR THIS FUNCTION. SET 0.0 TO ALL" << endl;
    }

    k = (kh + kv + (kh - kv) * pow(cos(deg2rad(elevationAngle)),2.0)*cos(2.0 * tau)) / 2.0;
    alpha = (kh*alphah + kv*alphav + (kh*alphah - kv*alphav) * pow(cos(deg2rad(elevationAngle)),2.0)*cos(2.0 * tau)) / (2.0 * k);

    rainLoss_dB = k * pow(weatherModel, alpha);

    return rainLoss_dB;
}

// Altshuler and Marr Model
double calculateCloudLoss(double frequency, double surfaceHumidity, double elevationAngleDeg, int cloudCover) {
    double wavelengthMm = (299792458.0 / frequency) / 1e-3; // Wavelength in millimeters
    double elevationAngleRad = deg2rad(elevationAngleDeg);

    // Effective Earth radius (km) and attenuating layer height (km)
    double effectiveEarthRadius = 8497.0;
    double attenuatingLayerHeight = 6.35 - 0.302 * surfaceHumidity;

    // Calculate D(theta) based on elevation angle
    double DTheta;
    if (elevationAngleDeg > 8.0) {
        if (elevationAngleDeg == 90.0) {  // Special case for 90 degrees to avoid divide-by-zero
            DTheta = attenuatingLayerHeight;
        } else {
            DTheta = 1.0 / sin(elevationAngleRad);  // Use std::csc if available
        }
    } else {
        DTheta = sqrt(pow(effectiveEarthRadius + attenuatingLayerHeight, 2) -
                         pow(effectiveEarthRadius * cos(elevationAngleRad), 2)) -
                 effectiveEarthRadius * sin(elevationAngleRad);
    }


    // Cloud attenuation for complete cloud cover (Ac)
    double Ac = (-0.0242 + 0.00075 * wavelengthMm + 0.403 / pow(wavelengthMm, 1.15)) *
                (11.3 + surfaceHumidity) * DTheta;

    // Adjust for partial cloud cover if needed
    if (cloudCover == 0){
        Ac *= 0.01;
    } else if (cloudCover == 1) {
        Ac *= 0.85;
    }
    // else keep whatever Ac already is

    return Ac;
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
