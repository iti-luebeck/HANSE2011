#ifndef MODULE_SONARLOCALIZATION_H
#define MODULE_SONARLOCALIZATION_H

#include <Framework/robotmodule.h>

class Module_ScanningSonar;

class Module_SonarLocalization : public RobotModule {
    Q_OBJECT

public:
    Module_SonarLocalization(QString id, Module_ScanningSonar* sonar);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);

private:
    Module_ScanningSonar* sonar;
};

#endif // MODULE_SONARLOCALIZATION_H
