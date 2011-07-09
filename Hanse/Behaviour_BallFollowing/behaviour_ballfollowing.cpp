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
    : RobotBehaviour(id), tracker(this)
{
    this->tcl = tcl;
    this->cams = cams;
    this->xsens = xsens;
    this->sim = sim;

    //    setEnabled(false);
    state = BALL_STATE_SEARCH_BALL;
    updateTimer.moveToThread(this);
    cutTimer.moveToThread(this);

    QObject::connect(cams, SIGNAL(newFrontImage(cv::Mat)), this, SLOT(newFrame(cv::Mat)));
}

void Behaviour_BallFollowing::init()
{
    active = false;
    setEnabled(false);
    logger->debug("ball init");

    connect(&updateTimer,SIGNAL(timeout()),this,SLOT(newData()));

    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(setForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));

    QObject::connect( xsens, SIGNAL(dataChanged(RobotModule*)),
                      this, SLOT(xsensUpdate(RobotModule*)) );

    connect(this, SIGNAL(requestFrame()), sim, SLOT(requestFrontImageSlot()));
    connect(sim, SIGNAL(newFrontImageData(cv::Mat)),this,SLOT(simFrame(cv::Mat)));


    setDefaultValue("kpBall", 0.6);
    setDefaultValue("deltaBall", 10);
    setDefaultValue("robCenterX", 320);
    setDefaultValue("robCenterY", 240);
    setDefaultValue("fwSpeed", 0.7);
    setDefaultValue("maxDistance", 240);
    setDefaultValue("threshold", 0);
    tracker.init();
}

bool Behaviour_BallFollowing::isActive()
{
    return active;
}

void Behaviour_BallFollowing::startBehaviour()
{
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    logger->debug( "Behaviour started" );

    tracker.reset();

    state = BALL_STATE_SEARCH_BALL;
    cutHeading = 0;

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }

    if (sim->isEnabled()) {
        updateTimer.start(500);
    } else {
        updateTimer.start(250);
    }
    emit started(this);
}

void Behaviour_BallFollowing::terminate()
{
    stop();
    RobotModule::terminate();
}

void Behaviour_BallFollowing::newData()
{
    if (!isActive()){
        return;
    }

    if (sim->isEnabled()) {
        emit requestFrame();
    }
}


void Behaviour_BallFollowing::newFrame(cv::Mat frame)
{
    if (!isActive()) {
        return;
    }

    this->dataLockerMutex.lock();
    frame.copyTo(this->frame);
    update();
    this->dataLockerMutex.unlock();
}

void Behaviour_BallFollowing::update()
{
    if (!isActive()){
        return;
    }

    if (!frame.empty()) {
        tracker.update(frame);
    }

    QString ballState = tracker.getState();
    double x = tracker.getMeanX();
    double area = tracker.getArea();
    double radius = std::sqrt(area / M_PI);
    double approxDistance = 0.15 * 326.7 / radius; // d = R * F / r (in m)

    if (state == BALL_STATE_SEARCH_BALL) {

        if (ballState == STATE_IS_SEEN) {
            double robCenterX = this->getSettingsValue("robCenterX").toDouble();
            double diffX = robCenterX - x;
            float angularSpeed = 0.0;

            if (approxDistance < 2) {
                if (fabs(diffX) < 2 * this->getSettingsValue("deltaBall").toFloat()) {
                    state = BALL_STATE_FOUND_BALL;
                } else {
                    // Ausrichten
                    angularSpeed = this->getSettingsValue("kpBall").toFloat() * (diffX / this->getSettingsValue("maxDistance").toFloat());
                }
                emit setAngularSpeed(angularSpeed);
                emit setForwardSpeed(0);
            } else {
                if (fabs(diffX) > this->getSettingsValue("deltaBall").toFloat()) {
                    angularSpeed = this->getSettingsValue("kpBall").toFloat() * (diffX / this->getSettingsValue("maxDistance").toFloat());
                }

                emit setAngularSpeed(angularSpeed);
                emit setForwardSpeed(this->getSettingsValue("fwSpeed").toFloat());
            }
        } else {
            emit setAngularSpeed(0.2);
            emit setForwardSpeed(0.3);
        }

    } else if (state == BALL_STATE_FOUND_BALL) {
        emit setAngularSpeed(0.0);
        emit setForwardSpeed(0.0);
    } else if (state == BALL_STATE_CUT_BALL) {

        if (ballState == STATE_IS_SEEN) {
            double robCenterX = this->getSettingsValue("robCenterX").toDouble();
            double diffX = robCenterX - x;
            float angularSpeed = 0.0;
            float forwardSpeed = 0.0;

            if (fabs(diffX) > this->getSettingsValue("deltaBall").toFloat()) {
                angularSpeed = this->getSettingsValue("kpBall").toFloat() * (diffX / this->getSettingsValue("maxDistance").toFloat());
            }

            if (fabs(diffX) < 2 * this->getSettingsValue("deltaBall").toFloat()) {
                forwardSpeed = this->getSettingsValue("fwSpeed").toFloat();
            }

            emit setAngularSpeed(angularSpeed);
            emit setForwardSpeed(forwardSpeed);
        } else if (ballState == STATE_PASSED) {
            state = BALL_STATE_DO_CUT;
//            cutHeading = xsens->getHeading();
//            emit setForwardSpeed(this->getSettingsValue("fwSpeed").toFloat());
//            cutTimer.singleShot(15000, this, SLOT(stopCut()));
        } else {
            // Do search for ball...
            float angularSpeed = 0.0;
            float forwardSpeed = 0.0;
            if (ballState == STATE_LOST_LEFT) {
                angularSpeed = -0.2;
                forwardSpeed = 0.4;
            } else if (ballState == STATE_LOST_RIGHT) {
                angularSpeed = 0.2;
                forwardSpeed = 0.4;
            } else if (ballState == STATE_LOST_BOTTOM) {
                state = BALL_STATE_DO_CUT;
                angularSpeed = 0.0;
                forwardSpeed = 0.4;
            } else if (ballState == STATE_LOST_TOP) {
                state = BALL_STATE_DO_CUT;
                angularSpeed = 0.0;
                forwardSpeed = 0.4;
            } else if (ballState == STATE_LOST) {
                // PANIC
                state = BALL_STATE_DO_CUT;
                angularSpeed = 0.0;
                forwardSpeed = 0.0f;
            }
            emit setAngularSpeed(angularSpeed);
            emit setForwardSpeed(forwardSpeed);
        }

    } else if (state == BALL_STATE_DO_CUT) {

    }

    emit newBallState(state);

    addData("state", state);
    addData("ball state", ballState);
    addData("x", x);
    addData("distance", approxDistance);
    addData("area", area);

    emit dataChanged(this);
}

void Behaviour_BallFollowing::xsensUpdate( RobotModule * )
{
    if (!isActive()) {
        return;
    }

    if (state == BALL_STATE_DO_CUT)
    {
        double currentHeading = xsens->getHeading();
        double diffHeading = cutHeading - currentHeading;
        float angularSpeed = 0.0;

        if (fabs(diffHeading) > 5) {
            angularSpeed = 0.01 * diffHeading;
        }

        emit setForwardSpeed(this->getSettingsValue("fwSpeed").toFloat());
        emit setAngularSpeed(angularSpeed);
    }
}

void Behaviour_BallFollowing::stop()
{
    if(!isActive()){
        logger->info("Not active!");
        return;
    }
    logger->debug( "Behaviour stopped" );
    active = false;
    setEnabled(false);

    state = BALL_STATE_SEARCH_BALL;
    updateTimer.stop();
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);
    emit finished(this,false);

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

    for ( int i = 0; i < files.count(); i++ )
    {
        QString filePath = path;
        filePath.append( "/" );
        filePath.append( files[i] );
        qDebug() << filePath;
        this->dataLockerMutex.lock();
        frame = imread( filePath.toStdString() );
        if (filePath.endsWith(".jpg")) {
            cvtColor(frame, frame, CV_RGB2BGR);
        }
        this->dataLockerMutex.unlock();

        update();
        msleep(500);
    }
    qDebug("done");
}

void Behaviour_BallFollowing::stopCut()
{
    if (!isActive()){
        return;
    }

    state = BALL_STATE_SEARCH_BALL;
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);
}

void Behaviour_BallFollowing::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate BallFollowing");
        stop();
    } else if(!enabled && !isActive()){
        logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        logger->info("Enable and activate BallFollowing");
        startBehaviour();
    } else {
        logger->info("Still activated");
    }
}

BallTracker *Behaviour_BallFollowing::getTracker()
{
    return &tracker;
}

void Behaviour_BallFollowing::setState(QString state)
{
    this->state = state;
}
