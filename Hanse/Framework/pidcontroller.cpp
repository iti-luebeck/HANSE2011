#include "pidcontroller.h"

PIDController::PIDController()
{
    setValues(0.0);

    y = 0;
//    QTimer::singleShot(100, this, SLOT(test()));
//    stest = 1.5;
}

void PIDController::setValues(double Kp, double Ti, double Td, double offset, double min, double max, double minHysteresis, double maxHysteresis, double minUpdateTime)
{
    this->Kp = Kp;
    this->Ti = Ti;
    this->Td = Td;

    this->offset = offset;
    this->min = min;
    this->max = max;

    this->minHysteresis = minHysteresis;
    this->maxHysteresis = maxHysteresis;

    this->minUpdateTime = minUpdateTime;
}

double PIDController::nextControlValue(double setpoint, double actual, bool &ok)
{
    double time = ((double)QDateTime::currentMSecsSinceEpoch()) / 1000;

    if (time > minUpdateTime) {
        time = minUpdateTime;
        ok = false;
    } else {
        ok = true;
    }

    double e = setpoint - actual;
    double u = 0;

    if (ticks > 1) {
        double deltaT = time - lastTime;
        u = lastu + Kp * ((1 + deltaT*Ti + Td/deltaT)*e - (1 + 2*Td/deltaT)*laste + (Td/deltaT)*lastlaste);
    }

    if (setpoint >= minHysteresis && setpoint <= maxHysteresis &&
        actual >= minHysteresis && actual <= maxHysteresis) {
        u = 0;
    }

    u = u + offset;
    if (u < min) {
        u = min;
    } else if (u > max) {
        u = max;
    }

    lastTime = time;
    lastu = u;
    lastlaste = laste;
    laste = e;
    ticks++;

    emit newData(setpoint, actual, u);

    return u;
}

void PIDController::reset()
{
    lastTime = 0;
    lastu = 0;
    laste = 0;
    lastlaste = 0;
    ticks = 0;
}

void PIDController::test()
{
    double time = ((double)QDateTime::currentMSecsSinceEpoch()) / 1000;
    double deltaT = time - lastTime;
    double T = 0.5;

    bool ok;
    double u = nextControlValue(stest, y, ok);
    if (ticks > 1) {
        // Simulation mit PT1-Strecke.
//        y = (1 / (T/deltaT + 1)) * (u + (T/deltaT)*y); // PT1

        // Simulation mit reiner I-Strecke, Auftrieb -> 0.1
        y = y + (deltaT / T) * (u - 0.1);
        if (y < 0) y = 0;
    }
    if (ticks == 200) stest = 0.5;
    if (ticks == 400) stest = 1.0;
    if (ticks == 600) stest = 0.1;
    if (ticks == 800) stest = 2.0;
    if (ticks == 1000) stest = 0.5;
    QTimer::singleShot(100, this, SLOT(test()));
}
