#ifndef MODULE_NAVIGATION_H
#define MODULE_NAVIGATION_H

#include <Framework/robotmodule.h>

class Module_Localization;
class Module_ThrusterControlLoop;

class Module_Navigation : public RobotModule
{
    Q_OBJECT
public:
    Module_Navigation(QString id, Module_Localization *localization, Module_ThrusterControlLoop* tcl);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);

protected:
    virtual void doHealthCheck();

private:
    Module_Localization *localization;
    Module_ThrusterControlLoop* tcl;

};
#endif // MODULE_NAVIGATION_H
