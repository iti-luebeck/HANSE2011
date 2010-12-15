#ifndef MODULE_ADC_H
#define MODULE_ADC_H

#include <QtCore>
#include <Framework/robotmodule_mt.h>
#include "inttypes.h"

class Module_UID;

class Module_ADC : public RobotModule_MT {
    Q_OBJECT

public:
    Module_ADC(QString id, Module_UID *uid);
    ~Module_ADC();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();


signals:
    void healthStatusChanged(HealthStatus data);
    void emergencyStop();

protected:
    virtual void doHealthCheck();

private slots:
    void refreshData();

private:
    Module_UID *uid;
    QTimer timer;
    QList<float> waterFilter;
    QList<float> voltage1;
    QList<float> voltage2;
    QList<float> voltage3;

    bool readChannel(int channel, float *value);
    void initFilter();

};

#endif // MODULE_ADC_H
