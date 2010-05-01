#ifndef MODULE_THRUSTERCONTROLLOOP_H
#define MODULE_THRUSTERCONTROLLOOP_H

#include "Module_ThrusterControlLoop_global.h"
#include "module_pressuresensor.h"
#include "module_thruster.h"

class MODULE_THRUSTERCONTROLLOOPSHARED_EXPORT Module_ThrusterControlLoop : public RobotModule {
    Q_OBJECT

public:
    Module_ThrusterControlLoop(QString id, Module_PressureSensor* pressure, Module_Thruster* thrusterLeft, Module_Thruster* thrusterRight, Module_Thruster* thrusterDown);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:

    /**
      * Reset control loops
      *
      * stop thrusters; return to surface
      */
    void reset();

    void terminate();

    /**
      * Set forward speed and hold it until told otherwise.
      *
      * range of speed: m/s ???
      */
    void setForwardSpeed(float speed);

    /**
      * Set angular speed and hold it until told otherwise.
      *
      * positive angular speed implies a clockwise rotation;
      * negative angular speed implies a counterclockwise rotation.
      * (both rotations as seem from above the robot)
      *
      * range: rad/s ???
      */
    void setAngularSpeed(float angularSpeed);

    /**
      * Dive to "depth" and stay there.
      *
      * depth: meters below the surface
      *        range: 0 to infinity
      */
    void setDepth(float depth);

private:

    Module_PressureSensor* pressure;
    Module_Thruster* thrusterLeft;
    Module_Thruster* thrusterRight;
    Module_Thruster* thrusterDown;

private slots:

    void newDepthData(float depth);

};

#endif // MODULE_THRUSTERCONTROLLOOP_H

