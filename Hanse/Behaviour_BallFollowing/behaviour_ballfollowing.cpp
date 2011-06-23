#include "behaviour_ballfollowing.h"

#include <QtGui>

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_Webcams/module_webcams.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Module_Simulation/module_simulation.h>

#include <Behaviour_BallFollowing/ballfollowingform.h>
#include <opencv/highgui.h>
#include <Framework/Angles.h>

using namespace cv;

Behaviour_BallFollowing::Behaviour_BallFollowing(QString id, Module_ThrusterControlLoop *tcl,
                                                 Module_Webcams *cams, Module_XsensMTi *xsens, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->cams = cams;
    this->xsens = xsens;
    this->sim = sim;

//    setEnabled(false);
    state = BALL_STATE_IDLE;
    updateTimer.moveToThread(this);
    timerNoBall.moveToThread(this);

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
}

void Behaviour_BallFollowing::init()
{
    connect(&updateTimer,SIGNAL(timeout()),this,SLOT(newData()));
    connect(&timerNoBall,SIGNAL(timeout()),this,SLOT(timerSlot()));

    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(setForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));

    // Wieso in der init -45 und im Behav.start nochmal?
    targetHeading = xsens->getHeading() - 45;
    targetHeading = Angles::deg2deg(targetHeading);

    QObject::connect( xsens, SIGNAL(dataChanged(RobotModule*)),
                      this, SLOT(xsensUpdate(RobotModule*)) );

    connect(this, SIGNAL(requestFrame()), sim, SLOT(requestFrontImageSlot()));
    connect(sim, SIGNAL(newFrontImageData(cv::Mat)),this,SLOT(simFrame(cv::Mat)));

    tracker.init();
}

bool Behaviour_BallFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_BallFollowing::startBehaviour()
{
    if (this->isEnabled() == true){
        logger->info("Already enabled/started!");
        return;
    }
    logger->debug( "Behaviour started" );
    this->setEnabled( true );
    tracker.reset();

    state = BALL_STATE_TURN_45;
    targetHeading = Angles::deg2deg(xsens->getHeading() - 45);

    emit setAngularSpeed(-0.4);

    updateTimer.start( 100 );
    emit started(this);
}

void Behaviour_BallFollowing::terminate()
{
    stop();
    RobotModule::terminate();
}

void Behaviour_BallFollowing::newData()
{
    if (this->isEnabled()) {

        if(sim->isEnabled()) {
            emit requestFrame();
        } else {
            cams->grabLeft(frame);
            update();
        }
    }
}


void Behaviour_BallFollowing::simFrame(cv::Mat simFrame)
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    this->dataLockerMutex.lock();
    simFrame.copyTo(frame);
    this->dataLockerMutex.unlock();

    update();
}

void Behaviour_BallFollowing::update()
{
    if (!this->isEnabled()){
        logger->info("Not enabled!");
        return;
    }

    tracker.update(frame);

    QString ballState = tracker.getState();
    double x = tracker.getMeanX();

    if (ballState == STATE_IS_SEEN) {
        double robCenterX = this->getSettingsValue("robCenterX").toDouble();
        double diffX = robCenterX - x;
        float angleSpeed = 0.0;

        if (fabs(diffX) > this->getSettingsValue("deltaBall").toFloat()) {
            angleSpeed = this->getSettingsValue("kpBall").toFloat() * (diffX / this->getSettingsValue("maxDistance").toFloat());
        }

        emit setAngularSpeed(angleSpeed);
        emit setForwardSpeed(this->getSettingsValue("fwSpeed").toFloat());
    } else if (ballState == STATE_PASSED) {
        emit setForwardSpeed(this->getSettingsValue("fwSpeed").toFloat());
        // Do xsens follow...
    } else {
        // Do search for ball...
    }

    ellipse(frame, RotatedRect(Point(x, tracker.getMeanY()), Size(std::sqrt(tracker.getArea()), std::sqrt(tracker.getArea())), 0.0f), Scalar(255,0,0), 5);
    addData("state", ballState);
    addData("x", x);

    emit dataChanged(this);
}

void Behaviour_BallFollowing::xsensUpdate( RobotModule * )
{
    if ( isEnabled() && state == BALL_STATE_TURN_45 )
    {
        double currentHeading = xsens->getHeading();
        double diffHeading = fabs( targetHeading - currentHeading );
        if ( diffHeading < 5 )
        {
            state = BALL_STATE_TRACK_BALL;
            emit setAngularSpeed(0.0);
            emit setForwardSpeed(0.0);
        }
    }
}

void Behaviour_BallFollowing::stop()
{
    logger->debug( "Behaviour stopped" );
    if (isEnabled()) {
        state = BALL_STATE_IDLE;
        updateTimer.stop();
        emit setForwardSpeed(0.0);
        emit setAngularSpeed(0.0);
        setEnabled(false);
        emit finished(this,false);
    }
}

void Behaviour_BallFollowing::reset()
{
    logger->debug( "Behaviour reset" );
    RobotBehaviour::reset();
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);
}

QList<RobotModule*> Behaviour_BallFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(cams);
    ret.append(xsens);
    ret.append(sim);
    return ret;
}

QWidget* Behaviour_BallFollowing::createView(QWidget* parent)
{
    return new BallFollowingForm(parent, this);
}

void Behaviour_BallFollowing::testBehaviour(QString path)
{
    QDir dir( path );
    dir.setFilter( QDir::Files );
    QStringList filters;
    filters << "*.jpg" << "*.png" << "*.bmp";
    dir.setNameFilters( filters );
    QStringList files = dir.entryList();

    namedWindow("Ball");
    for ( int i = 0; i < files.count(); i++ )
    {
        QString filePath = path;
        filePath.append( "/" );
        filePath.append( files[i] );
        frame = imread( filePath.toStdString() );

        update();
        imshow("Ball", tracker.getGray());

        cvWaitKey( 200 );
    }
}

void Behaviour_BallFollowing::timerSlot()
{
    if (!this->isEnabled()){
        logger->info("Not enabled!");
        return;
    }
    timerNoBall.stop();
    stop();
}

void Behaviour_BallFollowing::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(stop()));
    }
}
