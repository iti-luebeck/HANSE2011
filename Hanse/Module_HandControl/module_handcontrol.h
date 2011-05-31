#ifndef MODULE_HANDCONTROL_H
#define MODULE_HANDCONTROL_H

#include <Framework/robotbehaviour.h>

class Server;
class Module_Thruster;
class Module_ThrusterControlLoop;
class HandControl_Form;

class Module_HandControl : public RobotBehaviour {
    Q_OBJECT
    friend class HandControl_Form;

public:
    Module_HandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Thruster* thrusterLeft, Module_Thruster* thrusterRight, Module_Thruster* thrusterDown, Module_Thruster* thrusterDownFront);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

public slots:
    void startBehaviour();
    void stop();
    void reset();
    void terminate();


signals:
    void emergencyStop();
    void startHandControl();
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);
    void setDepth(float depth);
    void setUpDownSpeed(float upDownSpeed);
    void setLeftSpeed(float leftSpeed);
    void setRightSpeed(float rightSpeed);
    void stopServer();
    void startServer();

private:

    Module_Thruster* thrusterLeft;
    Module_Thruster* thrusterRight;
    Module_Thruster* thrusterDown;
    Module_Thruster* thrusterDownFront;
    Module_ThrusterControlLoop *controlLoop;


//    QAction *forward;
//    QAction *backward;
//    QAction *left;
//    QAction *right;
//    QAction *up;
//    QAction *down;

    Server* server;
    void init();

private slots:
    void sendNewControls();
    void newMessage(int forwardSpeed, int angularSpeed, int speedUpDown);
    void serverReportedError(QString error);
    void emergencyStopReceived();
    void startHandControlReceived();
};

#endif // MODULE_HANDCONTROL_H

