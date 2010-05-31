#ifndef MODULE_VISUALSLAM_H
#define MODULE_VISUALSLAM_H

#include <Framework/robotmodule.h>

class Module_SonarLocalization;

class Module_VisualSLAM : public RobotModule {
    Q_OBJECT

public:
    Module_VisualSLAM(QString id, Module_SonarLocalization* sonarLocalization);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);

private:
    Module_SonarLocalization* sonarLocalization;
};

#endif // MODULE_VISUALSLAM_H
