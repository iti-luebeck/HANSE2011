#ifndef MODULE_THRUSTER_H
#define MODULE_THRUSTER_H

#include <Framework/healthstatus.h>
#include <Framework/robotmodule.h>

class Module_UID;
class Module_Simulation;

class Module_Thruster : public RobotModule {
    Q_OBJECT

public:
    Module_Thruster(QString id, Module_UID *uid, Module_Simulation *sim);
    ~Module_Thruster();

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();
    void stop();

protected slots:

    virtual void doHealthCheck();
    void gotEnabled(bool value);

    /**
      * Set speed on thruster. Value ranges between -1 and 1.
      *
      * -1 to 0 --> rotate thruster backwards
      * 0 to 1 --> rotate thruster forwards
      *
      * 0 stops the thruster, while -1 and 1 move the thruster at maximum speed
      * in the aforementioned direction.
      */
    void setSpeed(float speed);


signals:
    void healthStatusChanged(HealthStatus data);
    void requestThrusterSpeed(QString id, int speed);

private:
    Module_UID *uid;
    Module_Simulation *sim;
    void initController();
    void init();

};

#endif // MODULE_THRUSTER_H
