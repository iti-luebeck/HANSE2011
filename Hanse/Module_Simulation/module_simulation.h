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

    void requestSonarSlot();
    void requestSonarGroundSlot();
    void requestImageSlot();
    void requestDepthSlot();
    void requestDepthWithNoiseSlot(int noise);
    void requestTempSlot();
    void requestTempWithNoiseSlot(int noise);
    void requestThrusterSpeedSlot(QString id,int speed);
    void requestAnglesSlot();
    void requestIMUSlot();

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
    void newSonarGroundData(SonarReturnData sondat);
    void newIMUData(float accl_x,float accl_y,float accl_z,float angvel_x,float angvel_y,float angvel_z);
    void newImageData(cv::Mat simframe);
    void test(char* readbuff);

protected:
    virtual void doHealthCheck();

private slots:
    void start();

private:
    bool client_running;
    QTimer timer;
    QThread thread;
    quint32 blockSize;
    int     read_ret;
    QTcpSocket *tcpSocket;
    //QDataStream input_stream2;

    void init();
    void parse_input(QString input);
    void connectToServer();

    void requestDepth();
    void requestDepthWithNoise(int noise);
    void requestTemp();
    void requestTempWithNoise(int noise);
    void requestThrusterSpeed(QString id,int speed);
    void requestAngles();
    void requestSonar();
    void requestSonarGround();
    void requestIMU();
    void requestImage();
};

#endif // MODULE_SIMULATION_H
