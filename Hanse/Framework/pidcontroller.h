#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include <limits>
#include <QtCore>

class PIDController : public QObject
{
    Q_OBJECT

public:
    PIDController();
    double nextControlValue(double setpoint, double actual, bool &ok);
    void reset();

public slots:
    void setValues(double Kp, double Ti = 0, double Td = 0, double offset = 0, double min = std::numeric_limits<double>::min(),
                   double max = std::numeric_limits<double>::max(), double minHysteresis = -1.0, double maxHysteresis = 0.2, double minUpdateTime = 1.0);

private:
    double ticks;

    double Kp;
    double Ti;
    double Td;

    double offset;
    double min;
    double max;

    double minHysteresis;
    double maxHysteresis;

    double minUpdateTime;

    double lastTime;
    double lastu;
    double laste;
    double lastlaste;

    double y;
    double stest;

signals:
    void newData(double setpoint, double actual, double u);

private slots:
    void test();
};

#endif // PIDCONTROLLER_H
