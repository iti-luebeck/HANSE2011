#ifndef MODULE_THRUSTERCONTROLLOOP_H
#define MODULE_THRUSTERCONTROLLOOP_H

#include <Framework/robotmodule_mt.h>

class Module_PressureSensor;
class Module_Thruster;

class Module_ThrusterControlLoop : public RobotModule_MT {
    Q_OBJECT

    friend class TCL_Form;
public:
    Module_ThrusterControlLoop(QString id, Module_PressureSensor* pressure, Module_Thruster* thrusterLeft, Module_Thruster* thrusterRight, Module_Thruster* thrusterDown, Module_Thruster* thrusterDownFront);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    /**
      * Copy control-loop-constants from Init-File (see form)
      * to the internal variables.
      *
      */
    void updateConstantsFromInitNow();

    /**
      * Return forward speed
      *
      * range of speed: -1.0 to 1.0
      */
    float getForwardSpeed();

    /**
      * Return current angular speed.
      *
      * positive angular speed implies a clockwise rotation;
      * negative angular speed implies a counterclockwise rotation.
      * (both rotations as seem from above the robot)
      *
      * range: -1.0 to 1.0
      */
    float getAngularSpeed();

    /**
      * Return "Soll-Tiefe"
      *
      * depth: meters below the surface
      *        range: 0 to infinity
      */
    float getDepth();

    float getDepthError();

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
      * range of speed: -1.0 to 1.0
      */
    void setForwardSpeed(float speed);

    /**
      * Set angular speed and hold it until told otherwise.
      *
      * positive angular speed implies a clockwise rotation;
      * negative angular speed implies a counterclockwise rotation.
      * (both rotations as seem from above the robot)
      *
      * range: -1.0 to 1.0
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

    void updateHorizontalThrustersNow();

    Module_PressureSensor* pressure;
    Module_Thruster* thrusterLeft;
    Module_Thruster* thrusterRight;
    Module_Thruster* thrusterDown;
    Module_Thruster* thrusterDownFront;

    // Control-Loop-Constants:

    // Speed of the UpDownThruster:
    // TODO: PRESUMPTION: speed>0.0 means UP
    float p_down;   // should be +
    float p_up;     // should be +
    float maxSpD;   // should be -
    float maxSpU;   // should be +
    float neutrSpD; // should be -
    float maxDepthError; // should be +

    bool horizSpM_exp;

    // Actual-Speed-Variables:
    float actualForwardSpeed;
    float actualAngularSpeed;

    float setvalueDepth;

    bool control_loop_enabled;
    bool ignoreHealth;
    bool pressureSensor_isHealthOK;

    static const int maxHist = 600;
    QMap<QDateTime,float> historyIst;
    QMap<QDateTime,float> historySoll;
    QMap<QDateTime,float> historyThrustCmd;

    volatile bool paused;

private slots:

    void newDepthData(float depth);
    void healthStatusChanged(HealthStatus pressureSensorHealth);


public slots:

    void pauseModule();
    void unpauseModule();
};

#endif // MODULE_THRUSTERCONTROLLOOP_H

