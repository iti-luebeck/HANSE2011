#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include <limits>
#include <QtCore>

class PIDController : public QObject
{
    Q_OBJECT

public:
    PIDController(double Kp, double Ti, double Td, double offset = 0, double min = std::numeric_limits<double>::min(), double max = std::numeric_limits<double>::max());
    double nextControlValue(double setpoint, double actual);
    void reset();

    void setValues(double Kp, double Ti, double Td, double offset = 0, double min = std::numeric_limits<double>::min(), double max = std::numeric_limits<double>::max());

private:
    double ticks;

    double Kp;
    double Ti;
    double Td;

    double offset;
    double min;
    double max;

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
