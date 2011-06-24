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
    return active;
}

void Behaviour_PipeFollowing::init()
{
    active = false;
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
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    this->reset();
    logger->info("Behaviour started" );
    Behaviour_PipeFollowing::updateFromSettings();
    this->setHealthToOk();

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }

    if (sim->isEnabled()) {
        timer.start(timerTime);
    } else {
        timer.start(2 * timerTime);
    }
    emit started(this);
}

void Behaviour_PipeFollowing::stop()
{
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    active = false;
    setEnabled(false);

    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    timer.stop();

        logger->info( "Behaviour stopped" );
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);

        emit finished(this,false);

}

void Behaviour_PipeFollowing::terminate()
{
    this->stop();
    RobotModule::terminate();

}

void Behaviour_PipeFollowing::reset()
{
    tracker.reset();
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
    if(!isActive()){
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
    if(!isActive()){
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
    if(!isActive()){
        return;
    }

    QTime run;
    run.restart();

    if (!frame.empty()) {
        this->setHealthToOk();
        tracker.update(frame);
        addData("time (msec)", run.elapsed());

        // Draw ideal line and robot center.
        line(frame, Point(frame.cols / 2, 0.0), Point(frame.cols / 2, frame.rows), Scalar(255,0,0), 3, 8);
        circle(frame, robCenter, 3, Scalar(255,0,255), 3, 8);
        // Draw pipe.
        line(frame, Point(tracker.getMeanX(), tracker.getMeanY()), Point(tracker.getMeanX() + 200 * sin(tracker.getOrientation()),
                                                                         tracker.getMeanY() - 200 * cos(tracker.getOrientation())), Scalar(255,0,0), 4, CV_FILLED);
        frame.copyTo(displayFrame);

        controlPipeFollow();
    } else {
        this->setHealthToSick("empty frame");
    }
}

void Behaviour_PipeFollowing::controlPipeFollow()
{
    if(!isActive()){
        return;
    }

    float ctrAngleSpeed = 0.0;
    float ctrForwardSpeed = 0.0;
    float curAngle = Angles::pi2deg(tracker.getOrientation());
    float distanceY = tracker.getDistanceToCenter();
    addData("pipe angle", curAngle);
    addData("pipe distance", distanceY);

    QString pipeState = tracker.getState();
    addData("pipe state", pipeState);

    if (pipeState == STATE_NOT_SEEN_YET) {
        // Assumption: Pipe is just ahead
        ctrAngleSpeed = 0.0;
        ctrForwardSpeed = constFWSpeed;
    } else if (pipeState == STATE_PASSED) {
        ctrAngleSpeed = 0.0;
        ctrForwardSpeed = 0.0f;
    } else if (pipeState == STATE_IS_SEEN) {
        if (fabs(curAngle) > Behaviour_PipeFollowing::deltaAngPipe) {
            ctrAngleSpeed = Behaviour_PipeFollowing::kpAngle * curAngle / 90.0;
        }

        if (fabs(distanceY) > Behaviour_PipeFollowing::deltaDistPipe) {
            ctrAngleSpeed += Behaviour_PipeFollowing::kpDist * distanceY / Behaviour_PipeFollowing::maxDistance;
        }

        ctrForwardSpeed = constFWSpeed;
    } else if (pipeState == STATE_LOST_LEFT) {
        ctrAngleSpeed = -0.2;
        ctrForwardSpeed = constFWSpeed;
    } else if (pipeState == STATE_LOST_RIGHT) {
        ctrAngleSpeed = 0.2;
        ctrForwardSpeed = constFWSpeed;
    } else if (pipeState == STATE_LOST_BOTTOM) {
        ctrAngleSpeed = 0.2;
        ctrForwardSpeed = 0.0f;
    } else if (pipeState == STATE_LOST_TOP) {
        ctrAngleSpeed = 0.0;
        ctrForwardSpeed = constFWSpeed;
    } else if (pipeState == STATE_LOST) {
        // PANIC
        ctrAngleSpeed = 0.0;
        ctrForwardSpeed = 0.0f;
    }

    emit angularSpeed(ctrAngleSpeed);
    emit forwardSpeed(ctrForwardSpeed);
    addData("speed angular", ctrAngleSpeed);
    addData("speed forward", ctrForwardSpeed);

    emit newPipeState(pipeState);
    emit dataChanged( this );
}

void Behaviour_PipeFollowing::analyzeVideo()
{
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
    if(!isActive()){
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
    if(!b && isActive()){
        logger->info("Disable and deactivate PipeFollowing");
        stop();
    } else if(!b && !isActive()){
        logger->info("Still deactivated");
    } else if(b && !isActive()){
        logger->info("Enable and activate PipeFollowing");
        startBehaviour();
    } else {
        logger->info("Still activated");
    }
}

void Behaviour_PipeFollowing::failed()
{
    // Bla
}
