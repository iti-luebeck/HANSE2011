#ifndef MODULE_SIMULATION_H
#define MODULE_SIMULATION_H

#define INITAL_BLOCK_SIZE 4

#include <QtCore>
#include <Framework/robotmodule.h>
#include "inttypes.h"
#include <QtNetwork>
#include <iostream>
#include <QTimer>
#include <opencv/cv.h>
#include "Module_ScanningSonar/sonarreturndata.h"
#include "Module_ScanningSonar/sonarswitchcommand.h"
#include "Module_EchoSounder/echoreturndata.h"
#include "Module_EchoSounder/echoswitchcommand.h"

class Module_Simulation : public RobotModule {
    Q_OBJECT

public:
    Module_Simulation(QString id);
    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

    void requestSonarSlot();
    void requestSonarGroundSlot();
    void requestSonarSideSlot();
    void requestFrontImageSlot();
    void requestBottomImageSlot();
    void requestDepthSlot();
    void requestTempSlot();
    void requestThrusterSpeedSlot(QString id,int speed);
    void requestAnglesSlot();
    void requestIMUSlot();
    void requestPingerSlot();

    void readResponse();
    void Hello_SIMAUV_Server();
    void sending_AUV_ID();
    void displayError(QAbstractSocket::SocketError socketError);

signals:
    void healthStatusChanged(HealthStatus data);
    /**
      * This signal is emmited whenever there a new messurement is available.
      *
      */
    void newDepthData(float depth);
    void newAngleData(float angle_yaw, float angle_pitch, float angle_roll);
    void newSonarData(SonarReturnData sondat);
    void newSonarGroundData(EchoReturnData echodat);
    void newSonarSideData(EchoReturnData echodat);
    void newIMUData(float accl_x,float accl_y,float accl_z,float angvel_x,float angvel_y,float angvel_z);
    void newBottomImageData(cv::Mat simframe);
    void newFrontImageData(cv::Mat simframe);
    void newPingerData(float angle);

protected:
    virtual void doHealthCheck();

private slots:
    void start();

private:
    bool client_running;
    QTimer timer;
    quint32 blockSize;
    int     read_ret;
    QTcpSocket *tcpSocket;

    void init();
    void parse_input(QString input);
    void connectToServer();

    void requestDepth();
    void requestTemp();
    void requestThrusterSpeed(QString id,int speed);
    void requestAngles();
    void requestSonar();
    void requestSonarGround();
    void requestSonarSide();
    void requestIMU();
    void requestFrontImage();
    void requestBottomImage();
    void requestPinger();
};

#endif // MODULE_SIMULATION_H
