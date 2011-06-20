#include "behaviour_pipefollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_PipeFollowing/pipefollowingform.h>
#include <opencv/cxcore.h>
#include <Module_Simulation/module_simulation.h>
#include <Framework/Angles.h>

Behaviour_PipeFollowing::Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Webcams *cam, Module_Simulation *sim) :
        RobotBehaviour(id), tracker(this)
{
    this->tcl = tcl;
    this->cam = cam;
    this->sim = sim;
    setEnabled(false);
    timer.moveToThread(this);

}

bool Behaviour_PipeFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_PipeFollowing::init()
{
    logger->debug("pipe init");
    connect(this,SIGNAL(forwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(angularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));

    frame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    displayFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    segmentationFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC1 );
    this->noPipeCnt = 0;

    this->updateFromSettings();
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));
    /* connect simulation */
    connect(this,SIGNAL(requestBottomFrame()),sim,SLOT(requestBottomImageSlot()));
    connect(sim,SIGNAL(newBottomImageData(cv::Mat)),this,SLOT(simFrame(cv::Mat)));

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));

    connect(&lostPipeTimer, SIGNAL(timeout()), this, SLOT(failed()));

}

void Behaviour_PipeFollowing::startBehaviour()
{
    if (this->isEnabled() == true){
        logger->info("Already enabled/started!");
        return;
    }

    this->reset();
    logger->info("Behaviour started" );
    Behaviour_PipeFollowing::updateFromSettings();
    this->setHealthToOk();
    setEnabled(true);
    timer.start(timerTime);
    emit started(this);
}

void Behaviour_PipeFollowing::stop()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    timer.stop();
    if (this->isActive())
    {
        logger->info( "Behaviour stopped" );
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
        setEnabled(false);
        emit finished(this,false);
    }
}

void Behaviour_PipeFollowing::terminate()
{
    this->stop();
    RobotModule::terminate();

}

void Behaviour_PipeFollowing::reset()
{
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
}

QList<RobotModule*> Behaviour_PipeFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(cam);
    ret.append(sim);
    return ret;
}

QWidget* Behaviour_PipeFollowing::createView(QWidget* parent)
{
    return new PipeFollowingForm( parent, this);
}

void Behaviour_PipeFollowing::simFrame(cv::Mat simFrame)
{
  //  logger->debug(" simu data");
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    //  QMutexLocker l(&this->dataLockerMutex);
    this->dataLockerMutex.lock();
    simFrame.copyTo(frame);
    this->dataLockerMutex.unlock();
//    imshow("blub",frame);
    timerSlotExecute();
}

void Behaviour_PipeFollowing::timerSlot()
{
    if (this->isEnabled() == false) {
        logger->info("Not enabled!");
        return;
    }

    if(sim->isEnabled()) {
        emit requestBottomFrame();
    } else {
        cam->grabBottom(frame);
        timerSlotExecute();
    }
}

void Behaviour_PipeFollowing::timerSlotExecute()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    QTime run;
    run.restart();

    if (!frame.empty()) {
        this->setHealthToOk();
        tracker.update(frame);
        addData("time image processing", run.elapsed());

        // Draw ideal line and robot center.
        line(frame, Point(frame.cols / 2, 0.0), Point(frame.cols / 2, frame.rows), Scalar(255,0,0), 3, 8);
        circle(frame, robCenter, 3, Scalar(255,0,255), 3, 8);
        // Draw pipe.
        line(frame, Point(tracker.getMeanX(), tracker.getMeanY()), Point(tracker.getMeanX() + 200 * sin(tracker.getPipeAngle()),
                                                                         tracker.getMeanY() - 200 * cos(tracker.getPipeAngle())), Scalar(255,0,0), 4, CV_FILLED);
        frame.copyTo(displayFrame);

        controlPipeFollow();
    } else {
        this->setHealthToSick("empty frame");
    }
}

void Behaviour_PipeFollowing::controlPipeFollow()
{
    if (this->isEnabled() == false) {
        logger->info("Not enabled!");
        return;
    }

    float ctrAngleSpeed = 0.0;
    float curAngle = Angles::pi2deg(tracker.getPipeAngle());
    float distanceY = tracker.getPipeDistance();
    addData("pipe angle", curAngle);
    addData("pipe distance", distanceY);

    QString pipeState = tracker.getPipeState();
    addData("pipe state", pipeState);
    if (pipeState == PIPE_STATE_NOT_SEEN_YET) {
        // Do search.
    } else if (pipeState == PIPE_STATE_PASSED) {
        // Emit succeeded signal.
    } else if (pipeState == PIPE_STATE_IS_SEEN) {
        if (fabs(curAngle) > Behaviour_PipeFollowing::deltaAngPipe) {
            ctrAngleSpeed = Behaviour_PipeFollowing::kpAngle * curAngle / 90.0;
        }

        if (fabs(distanceY) > Behaviour_PipeFollowing::deltaDistPipe) {
            ctrAngleSpeed += Behaviour_PipeFollowing::kpDist * distanceY / Behaviour_PipeFollowing::maxDistance;
        }

        emit angularSpeed(ctrAngleSpeed);
        emit forwardSpeed(this->constFWSpeed);
        addData("angular_speed",ctrAngleSpeed);
        addData("forward_speed",this->constFWSpeed);
    } else if (pipeState == PIPE_STATE_LOST_LEFT) {
        ctrAngleSpeed = -0.2;
        emit angularSpeed(ctrAngleSpeed);
        emit forwardSpeed(constFWSpeed);
        addData("angular_speed", ctrAngleSpeed);
        addData("forward_speed", constFWSpeed);
    } else if (pipeState == PIPE_STATE_LOST_RIGHT) {
        ctrAngleSpeed = 0.2;
        emit angularSpeed(ctrAngleSpeed);
        emit forwardSpeed(constFWSpeed);
        addData("angular_speed", ctrAngleSpeed);
        addData("forward_speed", constFWSpeed);
    } else if (pipeState == PIPE_STATE_LOST_BOTTOM) {
        ctrAngleSpeed = 0.2;
        emit angularSpeed(ctrAngleSpeed);
        emit forwardSpeed(0);
        addData("angular_speed", ctrAngleSpeed);
        addData("forward_speed", 0);
    } else if (pipeState == PIPE_STATE_LOST_TOP) {
        ctrAngleSpeed = 0.0;
        emit angularSpeed(ctrAngleSpeed);
        emit forwardSpeed(constFWSpeed);
        addData("angular_speed", ctrAngleSpeed);
        addData("forward_speed", constFWSpeed);
    } else if (pipeState == PIPE_STATE_LOST) {
        // PANIC
    }

    emit dataChanged( this );
}

void Behaviour_PipeFollowing::analyzeVideo()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    QString videoFile = getSettingsValue("video directory").toString();

    QDir dir( videoFile );
    dir.setFilter( QDir::Files );
    QStringList filters;
    filters << "*.jpg";
    dir.setNameFilters( filters );
    files = dir.entryList();

    for ( int i = 0; i < files.count(); i++ )
    {
        QString filePath = videoFile;
        filePath.append( "/" );
        filePath.append( files[i] );
        frame = imread( filePath.toStdString() );

        timerSlotExecute();

        msleep(2000);
    }
}

void Behaviour_PipeFollowing::updateFromSettings()
{
    logger->debug("update Settings pipefollow");
    this->dataLockerMutex.lock();
//    this->firstRun = 1;
    this->timerTime = this->getSettingsValue("timer",0).toInt();
    this->threshSegmentation = this->getSettingsValue("threshold",188).toInt();
    this->debug = this->getSettingsValue("debug",0).toInt();
    this->deltaAngPipe = this->getSettingsValue("deltaAngle",11).toFloat();
    this->deltaDistPipe = this->getSettingsValue("deltaDist",100).toFloat();
    this->kpDist = this->getSettingsValue("kpDist",1).toFloat();
    this->kpAngle = this->getSettingsValue("kpAngle",1).toFloat();
    this->constFWSpeed = this->getSettingsValue("fwSpeed",0.8).toFloat();
    this->robCenter = Point(this->getSettingsValue("robCenterX",320).toDouble(),this->getSettingsValue("robCenterY",240).toDouble());
    this->maxDistance = this->getSettingsValue("maxDistance",320).toFloat();
    this->dataLockerMutex.unlock();
}

void Behaviour_PipeFollowing::grabFrame(cv::Mat &frame)
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    dataLockerMutex.lock();
    if(getSettingsValue("frameOutput").toBool()) {
        displayFrame.copyTo(frame);
    } else {
        cv::Mat tmp(WEBCAM_HEIGHT,WEBCAM_WIDTH,CV_8UC3);
        cvtColor(segmentationFrame,tmp,CV_GRAY2RGB);
        tmp.copyTo(frame);
    }
    dataLockerMutex.unlock();
}

void Behaviour_PipeFollowing::setUpdatePixmapSlot(bool bol){
    emit setUpdatePixmapSignal(bol);
}

void Behaviour_PipeFollowing::controlEnabledChanged(bool b){
    if (b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(stop()));
    }
}

void Behaviour_PipeFollowing::failed()
{
    // Bla
}
