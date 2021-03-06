#include "module_simulation.h"
#include "simulation_form.h"
#include <QtCore>

Module_Simulation::Module_Simulation(QString id)
    : RobotModule(id)
{
    client_running = false;

    setDefaultValue("server_ip_adress", "localhost");
    setDefaultValue("server_port", 80);
    setDefaultValue("auv_id", "hanse");
    blockSize = 0;

    qRegisterMetaType<cv::Mat>("cv::Mat");
    timer.moveToThread(this);
}

void Module_Simulation::init()
{
    reset();
}

void Module_Simulation::terminate()
{
    timer.stop();
    RobotModule::terminate();
}

void Module_Simulation::reset()
{
    RobotModule::reset();

    if (isEnabled())
        this->start();
}

QList<RobotModule*> Module_Simulation::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* Module_Simulation::createView(QWidget* parent)
{
    return new Simulation_Form(this, parent);
}

void Module_Simulation::doHealthCheck()
{
    if (!isEnabled())
        return;

    setHealthToOk();
}

void Module_Simulation::start()
{
    logger->debug("Client start...");
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readResponse()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(Hello_SIMAUV_Server()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError))); 
    this->connectToServer();
}

void Module_Simulation::connectToServer()
{
    logger->debug("Aborting old connections...");
    tcpSocket->abort();
    QString debug_string = QString("Connecting to host: ");
    debug_string.append(getSettingsValue("server_ip_adress").toString()).append(":").append(getSettingsValue("server_port").toString());
    logger->debug(debug_string);
    tcpSocket->connectToHost(getSettingsValue("server_ip_adress").toString(),getSettingsValue("server_port").toInt());
}

void Module_Simulation::Hello_SIMAUV_Server()
{
    logger->debug("Sending HELLO to host...");
    tcpSocket->write(QString("HELLO SIMAUV SERVER\n").toAscii().data(), QString("HELLO SIMAUV SERVER\n").length());
    tcpSocket->flush();
}

void Module_Simulation::sending_AUV_ID()
{
    logger->debug("Sending AUV_ID to host...");
    QString request = QString("AUV_ID ").append(getSettingsValue("auv_id").toString()).append("\n");
    tcpSocket->write(request.toAscii().data(), request.length());
    tcpSocket->flush();
}

void Module_Simulation::requestDepth()
{
    if(client_running){
        QString request = QString("Depth ").append("press").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestTemp()
{
    if(client_running){
        QString request = QString("Temp ").append("temp").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestThrusterSpeed(QString id,int speed)
{
    if(client_running){
        addData(id,speed);
//        emit dataChanged(this);
        QVariant tmp(speed);
        QString request = QString("Thruster ").append(id).append(" ").append(tmp.toString()).append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestAngles()
{
    if(client_running){
        QString request = QString("Angles ").append("compass").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestSonar()
{
    if(client_running){
        QString request = QString("Sonar360 ").append("sonar_360").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestSonarGround()
{
    if(client_running){
        QString request = QString("SonarEcho ").append("sonar_ground").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestSonarSide()
{
    if(client_running){
        QString request = QString("SonarEcho ").append("sonar_side").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestIMU()
{
    if(client_running){
        QString request = QString("IMU\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestBottomImage()
{
    if(client_running){
        QString request = QString("Camera ").append("bottom_cam").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestFrontImage()
{
    if(client_running){
        QString request = QString("Camera ").append("front_cam").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestPinger()
{
    if(client_running){
        QString request = QString("Pinger ").append("ping").append("\n");
        tcpSocket->write(request.toAscii().data(), request.length());
        tcpSocket->flush();
    }
}

void Module_Simulation::requestBottomImageSlot(){
    requestBottomImage();
}

void Module_Simulation::requestFrontImageSlot(){
    requestFrontImage();
}

void Module_Simulation::requestSonarSlot(){
    requestSonar();
}

void Module_Simulation::requestSonarGroundSlot(){
    requestSonarGround();
}

void Module_Simulation::requestSonarSideSlot(){
    requestSonarSide();
}

void Module_Simulation::requestDepthSlot(){
    requestDepth();
}

void Module_Simulation::requestTempSlot(){
    requestTemp();
}

void Module_Simulation::requestThrusterSpeedSlot(QString id,int speed){
    requestThrusterSpeed(id,speed);
}

void Module_Simulation::requestAnglesSlot(){
    requestAngles();
}

void Module_Simulation::requestIMUSlot(){
    requestIMU();
}

void Module_Simulation::requestPingerSlot(){
    requestPinger();
}

void Module_Simulation::parse_input(QString input){
    QDataStream input_stream2(tcpSocket);
    if(input.startsWith("Depth"))
    {
        QString depth_name;
        input_stream2 >> depth_name;
        //logger->debug(depth_name);

        QString inputdata;
        input_stream2 >> inputdata;
        //logger->debug(inputdata);

        float ff = inputdata.toFloat();

        /*QString debug_string = QString("received data: ");
        QVariant tmp(ff);
        debug_string.append(tmp.toString());
        logger->debug(debug_string);*/

        addData("depth", (-1)*ff);
        emit newDepthData((-1)*ff);
    }
    else if(input.startsWith("Temp"))
    {
        QString temp_name;
        input_stream2 >> temp_name;

        QString inputdata;
        input_stream2 >> inputdata;

        float ff = inputdata.toFloat();
        addData("temperature", (10)*ff);
    }
    else if(input.startsWith("Camera"))
    {
        QString cam_name;
        input_stream2 >> cam_name;
//        logger->debug(cam_name);

        QByteArray inputdata;
        input_stream2 >> inputdata;

        char *datas = inputdata.data();

        if(cam_name.startsWith("bottom_cam")){
            int rows = 480;
            int cols = 640;
            cv::Mat* mat = new cv::Mat(rows, cols, CV_8UC4,datas);
            cv::Mat img_yuv;
            cv::Mat img_yuv2;
            cv::cvtColor(*mat,img_yuv,CV_RGBA2RGB);
            cv::flip(img_yuv,img_yuv2,1);
            emit newBottomImageData(img_yuv2);
        }else if(cam_name.startsWith("front_cam")){
            int rows = 480;
            int cols = 640;
            cv::Mat* mat = new cv::Mat(rows, cols, CV_8UC4,datas);
            cv::Mat img_yuv;
            cv::Mat img_yuv2;
            cv::cvtColor(*mat,img_yuv,CV_RGBA2RGB);
            cv::flip(img_yuv,img_yuv2,1);
            emit newFrontImageData(img_yuv2);
        }

    }
    else if(input.startsWith("Thruster"))
    {
        QString thruster_name;
        input_stream2 >> thruster_name;

        QString inputdata;
        input_stream2 >> inputdata;
    }
    else if(input.startsWith("Angles"))
    {
        QString compass_name;
        input_stream2 >> compass_name;

        QString input2;
        input_stream2 >> input2;

        QString qs = input2.section(' ',1,1);
        float angle_yaw = qs.toFloat();
        addData("heading", angle_yaw);
        QString qs2 = input2.section(' ',2,2);
        float angle_pitch = qs2.toFloat();
        addData("pitch", angle_pitch);
        QString qs3 = input2.section(' ',3,3);
        float angle_roll = qs3.toFloat();
        addData("roll", angle_roll);

        emit newAngleData(angle_yaw,angle_pitch,angle_roll);
    }
    else if(input.startsWith("Sonar360"))
    {
        QString sonar_name;
        input_stream2 >> sonar_name;

        QByteArray inputdata;
        input_stream2 >> inputdata;

        SonarSwitchCommand cmd;
        cmd.range = 50;
        inputdata[7] = 50;
        cmd.startGain = 1;
        cmd.stepSize = 1;
        cmd.dataPoints = 25;
        /*
        cmd.range = parent.getSettings().value("range").toInt();
        cmd.startGain = parent.getSettings().value("gain").toInt();
        cmd.trainAngle = parent.getSettings().value("trainAngle").toInt();
        cmd.sectorWidth = parent.getSettings().value("sectorWidth").toInt();
        cmd.stepSize = parent.getSettings().value("stepSize").toInt();
        cmd.pulseLength = parent.getSettings().value("pulseLength").toInt();
        cmd.dataPoints = parent.getSettings().value("dataPoints").toInt();
        cmd.switchDelay = parent.getSettings().value("switchDelay").toInt();
        cmd.frequency = parent.getSettings().value("frequency").toInt();
        */

        SonarReturnData* sondat = new SonarReturnData(cmd,inputdata);

        emit newSonarData(*sondat);

    }else if(input.startsWith("SonarEcho"))
    {
        QString sonar_name;
        input_stream2 >> sonar_name;

        QByteArray inputdata;
        input_stream2 >> inputdata;

        EchoSwitchCommand cmd;
        cmd.range = 5;
        inputdata[7] = 5;
        cmd.startGain = 1;
        cmd.dataPoints = 25;
        /*cmd.time = other.time;
        this->range = other.range;
        this->startGain = other.startGain;
        this->pulseLength = other.pulseLength;
        this->profileMinRange = other.profileMinRange;
        this->profile = other.profile;
        this->frequency = other.frequency;
        this->switchDelay = other.switchDelay;
        this->totalBytes = other.totalBytes;
        this->nToRead = other.nToRead;
        this->origFileHeader = other.origFileHeader;*/

        EchoReturnData* echodat = new EchoReturnData(cmd,inputdata);
        if(sonar_name.startsWith("sonar_ground")){
            emit newSonarGroundData(*echodat);
        }else if(sonar_name.startsWith("sonar_side")){
            emit newSonarSideData(*echodat);
        }
    }
    else if(input.startsWith("IMU"))
    {
        QString imu_name;
        input_stream2 >> imu_name;

        QString input2;
        input_stream2 >> input2;

        QString qs2 = input2.section(' ',1,1);
        float accelX = qs2.toFloat();
        addData("accelX", accelX);

        QString qs3 = input2.section(' ',2,2);
        float accelY = qs3.toFloat();
        addData("accelY", accelY);

        QString qs4 = input2.section(' ',3,3);
        float accelZ = qs4.toFloat();
        addData("accelZ", accelZ);

        QString qs5 = input2.section(' ',4,4);
        float angvel_x = qs5.toFloat();
        addData("gyroX", angvel_x);

        QString qs6 = input2.section(' ',5,5);
        float angvel_y = qs6.toFloat();
        addData("gyroY", angvel_y);

        QString qs7 = input2.section(' ',6,6);
        float angvel_z = qs7.toFloat();
        addData("gyroZ", angvel_z);

        addData("gyroTempX", 0);
        addData("gyroTempY", 0);
        addData("gyroTempZ", 0);

        emit newIMUData(accelX,accelY,accelZ,angvel_x,angvel_y,angvel_z);
    }
    else if(input.startsWith("Pinger"))
    {
        QString pinger_name;
        input_stream2 >> pinger_name;

        QString input2;
        input_stream2 >> input2;

        QString qs = input2.section(' ',1,1);
        float angle = qs.toFloat();
        addData("angle", angle);

        emit newPingerData(angle);
    }
    else if(input.startsWith("HELLO"))
    {
        QString inputdata;
        input_stream2 >> inputdata;

        logger->debug("Received HELLO from host...");
        sending_AUV_ID();
    }
    else if(input.startsWith("AUV_ID"))
    {
        QString inputdata;
        input_stream2 >> inputdata;

        logger->debug(QString("Received AUV_ID ").append(inputdata).append(" from host..."));
        client_running = true;
        logger->debug("Allow sending of normal requests...");
    }
    else if(input.startsWith("AUV_NOT_EXIST"))
    {
        logger->debug(QString("The AUV_ID doesnt exist..."));
    }
}

void Module_Simulation::readResponse()
{
    //qDebug("Client read...");
    QMutexLocker l(&this->dataLockerMutex);

    QDataStream input_stream2(tcpSocket);
    input_stream2.setVersion(QDataStream::Qt_4_7);

    //process all new readable data and dont do it only once because of the readyRead() signal
    //that comes not regularly
    while(1){
        if (blockSize == 0) {//get the length of the following data
            if (tcpSocket->bytesAvailable() < (int)sizeof(quint32))
                return;

            input_stream2 >> blockSize;
            //QVariant tmp(blockSize);
            //logger->debug(tmp.toString());
        }

        //only proceed if we have enough data
        if (tcpSocket->bytesAvailable() < blockSize)
            return;

        //we have enugh data, start parsing
        QString command;
        input_stream2 >> command;
        //logger->debug(command);

        parse_input(command);

        //reset so we can take new commands
        blockSize = 0;
    }
}

void Module_Simulation::displayError(QAbstractSocket::SocketError Error)
{
    switch (Error) {
    case QAbstractSocket::RemoteHostClosedError:
        logger->debug("RemoteHostClosedError...");
        setHealthToSick("RemoteHostClosedError...");
        break;
    case QAbstractSocket::HostNotFoundError:
        logger->debug("HostNotFoundError...");
        setHealthToSick("HostNotFoundError...");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        logger->debug("ConnectionRefusedError...");
        setHealthToSick("ConnectionRefusedError...");
        break;
    default:
        logger->debug(QString(tcpSocket->errorString()));
        setHealthToSick(QString(tcpSocket->errorString()));
        break;
    }
}
