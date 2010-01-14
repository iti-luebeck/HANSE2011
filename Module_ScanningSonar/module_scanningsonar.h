#ifndef MODULE_SCANNINGSONAR_H
#define MODULE_SCANNINGSONAR_H

#include "Module_ScanningSonar_global.h"
#include "robotmodule.h"
#include <qextserialport.h>
#include <QTimer>

class MODULE_SCANNINGSONARSHARED_EXPORT Module_ScanningSonar : public RobotModule {
    Q_OBJECT

public:
    Module_ScanningSonar(QString id);
    ~Module_ScanningSonar();

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();

private slots:
    void doNextScan();

public slots:
    void reset();
    //void enabled(bool value);

private:
    QextSerialPort port;
    QTimer timer;
    void configurePort();
    QByteArray buildSwitchDataCommand();
};

#endif // MODULE_SCANNINGSONAR_H
