#ifndef MODULE_SIMULATION_H
#define MODULE_SIMULATION_H

#include <QtCore>
#include <Framework/robotmodule.h>
#include "inttypes.h"
#include <QtNetwork>
#include <iostream>
#include <QTimer>

class Module_UID;

class Module_Simulation : public RobotModule {
    Q_OBJECT

public:
    Module_Simulation(QString id);
    ~Module_Simulation();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);

protected:
    virtual void doHealthCheck();

private slots:
    void readFortune();
    void requestNewFortune();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    QTimer timer;
    QThread thread;
    unsigned int counter;
    quint16 blockSize;
    char    buf[512];
    int     read_ret;
    QString currentFortune;
    QTcpSocket *tcpSocket;
    void start();
};

#endif // MODULE_SIMULATION_H
