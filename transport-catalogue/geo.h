#pragma once

#include <cmath>

namespace transport_catalogue {

    struct Coordinates {
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    inline double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static int kGroundRadius = 6371000;
        static double kPi  = 3.1415926535;
        static const double kDr = kPi / 180.;
        return acos(sin(from.lat * kDr) * sin(to.lat * kDr)
                    + cos(from.lat * kDr) * cos(to.lat * kDr) * cos(abs(from.lng - to.lng) * kDr))
               * kGroundRadius;
    }
}
