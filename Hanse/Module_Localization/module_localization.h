#ifndef MODULE_LOCALIZATION_H
#define MODULE_LOCALIZATION_H

#include <Framework/robotmodule.h>

class Module_VisualSLAM;
class Module_SonarLocalization;

class Module_Localization : public RobotModule
{
    Q_OBJECT
public:
    Module_Localization(QString id, Module_VisualSLAM* visualSLAM, Module_SonarLocalization *sonarLoc);

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
    Module_VisualSLAM *visualSLAM;
    Module_SonarLocalization *sonarLocalization;
};

#endif // MODULE_LOCALIZATION_H
