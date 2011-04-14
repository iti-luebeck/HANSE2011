#ifndef ANGLES_H
#define ANGLES_H

#include <cmath>

class Angles {

public:
    static float pi2pi(float rad) {
        while (rad > M_PI) {
            rad -= 2*M_PI;
        }
        while (rad <= -M_PI) {
            rad += 2*M_PI;
        }
        return rad;
    }

    static float deg2deg(float deg) {
        while (deg > 180) {
            deg -= 360;
        }
        while (deg <= -180) {
            deg += 360;
        }
        return deg;
    }

    static float deg2pi(float deg) {
        float rad = M_PI * deg / 180;
        return pi2pi(rad);
    }

    static float pi2deg(float rad) {
        float deg = 180 * rad / M_PI;
        return deg2deg(deg);
    }

};

#endif // ANGLES_H
