#include "behaviour_ballfollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_BallFollowing/ballfollowingform.h>
#include <Behaviour_BallFollowing/blobs/blob.h>
#include <Behaviour_BallFollowing/blobs/BlobResult.h>
#include <opencv/highgui.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Framework/Angles.h>

Behaviour_BallFollowing::Behaviour_BallFollowing(QString id, Module_ThrusterControlLoop *tcl,
                                                 Module_Webcams *cams, Module_XsensMTi *xsens)
                                                     : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->cams = cams;
    this->xsens = xsens;

    setEnabled(false);
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
    state = BALL_STATE_TURN_45;
    targetHeading = xsens->getHeading() - 45;
    targetHeading = Angles::deg2deg(targetHeading);

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
    if( this->isEnabled() && state == BALL_STATE_TRACK_BALL )
    {
        Behaviour_BallFollowing::ctrBallFollowing();
    }
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
    ret.append( cams );
    ret.append( xsens );
    return ret;
}

QWidget* Behaviour_BallFollowing::createView(QWidget* parent)
{
    return new BallFollowingForm(parent, this);
}

void Behaviour_BallFollowing::testBehaviour( QString path )
{
    if (!this->isEnabled()){
        logger->info("Not enabled!");
        return;
    }

    QDir dir( path );
    dir.setFilter( QDir::Files );
    QStringList filters;
    filters << "*.jpg";
    dir.setNameFilters( filters );
    QStringList files = dir.entryList();

    cvNamedWindow("Dummy");
    for ( int i = 0; i < files.count(); i++ )
    {
        IplImage *gray = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 1 );
        IplImage *thresh = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 1 );
        IplImage *disp = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
        IplImage *hsv = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
        cvMerge( thresh, thresh, thresh, NULL, disp );

        QString filePath = path;
        filePath.append( "/" );
        filePath.append( files[i] );
        IplImage *left = cvLoadImage( filePath.toStdString().c_str(), 1 );

        // Apply threshold.
        cvCvtColor( left, gray, CV_RGB2GRAY );
        cvCvtColor( left, hsv, CV_RGB2HSV );
        cvSplit( hsv, NULL, gray, NULL, NULL );
        cvThreshold( gray, thresh, getSettingsValue( "threshold", 100 ).toDouble(), 255, CV_THRESH_BINARY );
        cvMerge( thresh, thresh, thresh, NULL, disp );

        // Do blob filtering.
        CBlobResult blobs( thresh, NULL, 255 );
        blobs.Filter( blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 500 );

        // Get largest blobs x position.
        float x = -1;
        int maxArea = 0;
        int maxBlob = -1;
        for ( int j = 0; j < blobs.GetNumBlobs(); j++ )
        {
            CBlob *blob = blobs.GetBlob( j );
            if ( blob->Area() > maxArea )
            {
                maxArea = blob->Area();
                maxBlob = j;
            }
        }
        if ( maxBlob >= 0 )
        {
            x = blobs.GetBlob( maxBlob )->Moment( 1, 0 ) / blobs.GetBlob( maxBlob )->Moment( 0, 0 );
            blobs.GetBlob( maxBlob )->FillBlob( left, cvScalar( 0, 0, 255 ), 0, 0 );
        }

        emit printFrame( left );

        cvReleaseImage( &left );
        cvReleaseImage( &gray );
        cvReleaseImage( &thresh );
        cvReleaseImage( &disp );
        cvReleaseImage( &hsv );

        cvWaitKey( 200 );
    }
}

void Behaviour_BallFollowing::ctrBallFollowing()
{
    if (!this->isEnabled()){
        logger->info("Not enabled!");
        return;
    }
    IplImage *gray = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 1 );
    IplImage *thresh = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 1 );
    IplImage *disp = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    IplImage *hsv = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    cvMerge( thresh, thresh, thresh, NULL, disp );
    IplImage *left = cvCreateImage( cvSize(WEBCAM_WIDTH,WEBCAM_HEIGHT), IPL_DEPTH_8U, 3 );
    cams->grabLeft( left );

    // Apply threshold.
    cvCvtColor( left, gray, CV_RGB2GRAY );
    cvCvtColor( left, hsv, CV_RGB2HSV );
    cvSplit( hsv, NULL, gray, NULL, NULL );
    cvThreshold( gray, thresh, getSettingsValue( "threshold", 100 ).toDouble(), 255, CV_THRESH_BINARY );
    cvMerge( thresh, thresh, thresh, NULL, disp );

    // Do blob filtering.
    CBlobResult blobs( thresh, NULL, 255 );
    blobs.Filter( blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 500 );

    // Get largest blobs x position.
    float x = -1;
    int maxArea = 0;
    int maxBlob = -1;
    for ( int j = 0; j < blobs.GetNumBlobs(); j++ )
    {
        CBlob *blob = blobs.GetBlob( j );
        if ( blob->Area() > maxArea )
        {
            maxArea = blob->Area();
            maxBlob = j;
        }
    }
    if ( maxBlob >= 0 )
    {
        x = blobs.GetBlob( maxBlob )->Moment( 1, 0 ) / blobs.GetBlob( maxBlob )->Moment( 0, 0 );
        blobs.GetBlob( maxBlob )->FillBlob( left, cvScalar( 0, 0, 255 ), 0, 0 );
    }

    emit printFrame( left );

    if ( x > 0 )
    {
        timerNoBall.stop();
        float robCenterX = this->getSettingsValue("robCenterX").toFloat();
        float diff = robCenterX - x;
        float angleSpeed = 0.0;
        diff < 0.0 ? diff *= (-1) : diff;
        if(diff > this->getSettingsValue("deltaBall").toFloat())
        {
            angleSpeed = this->getSettingsValue("kpBall").toFloat() * ((robCenterX - x)/this->getSettingsValue("maxDistance").toFloat());
        }
        emit setAngularSpeed(angleSpeed);
        emit setForwardSpeed(this->getSettingsValue("fwSpeed").toFloat());

        addData("ball_area", maxArea);
        addData("ball_x", x);
        addData("position_difference", diff);
        addData("angular_speed", angleSpeed);
        addData("forward_speed", this->getSettingsValue("fwSpeed").toFloat());
        dataChanged( this );
        timerNoBall.start( 60000 );
    }
    else
    {
        emit setAngularSpeed(0.0);
        emit setForwardSpeed(0.6);
    }

    cvReleaseImage( &left );
    cvReleaseImage( &gray );
    cvReleaseImage( &thresh );
    cvReleaseImage( &hsv );
    cvReleaseImage( &left );
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
