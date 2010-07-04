#include "behaviour_ballfollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_BallFollowing/ballfollowingform.h>
#include <Behaviour_BallFollowing/blobs/blob.h>
#include <Behaviour_BallFollowing/blobs/BlobResult.h>
#include <opencv/highgui.h>
#include <Module_Compass/module_compass.h>

Behaviour_BallFollowing::Behaviour_BallFollowing(QString id, Module_ThrusterControlLoop *tcl,
                                                 Module_Webcams *cams, Module_Compass *compass)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->cams = cams;
    this->compass = compass;

    connect(&updateTimer,SIGNAL(timeout()),this,SLOT(newData()));
    connect(&timerNoBall,SIGNAL(timeout()),this,SLOT(timerSlot()));

    setEnabled(false);
    state = STATE_IDLE;

    targetHeading = compass->getHeading() - 45;
    if ( targetHeading < 0 )
    {
        targetHeading += 360;
    }
    QObject::connect( compass, SIGNAL(dataChanged(RobotModule*)),
                      this, SLOT(compassUpdate(RobotModule*)) );
}

bool Behaviour_BallFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_BallFollowing::start()
{
    logger->debug( "Behaviour started" );
    this->setEnabled( true );
    state = STATE_TURN_45;
    targetHeading = compass->getHeading() - 45;
    if ( targetHeading < 0 )
    {
        targetHeading += 360;
    }
    tcl->setAngularSpeed( -0.4 );

    updateTimer.start( 100 );
}

void Behaviour_BallFollowing::newData()
{
    if( this->isEnabled() && state == STATE_TRACK_BALL )
    {
        Behaviour_BallFollowing::ctrBallFollowing();
    }
}

void Behaviour_BallFollowing::compassUpdate( RobotModule * )
{
    if ( isEnabled() && state == STATE_TURN_45 )
    {
        double currentHeading = compass->getHeading();
        double diffHeading = fabs( targetHeading - currentHeading );
        if ( diffHeading < 5 )
        {
            state = STATE_TRACK_BALL;
            tcl->setAngularSpeed( .0 );
            tcl->setForwardSpeed( .0 );
        }
    }
}

void Behaviour_BallFollowing::stop()
{
    logger->debug( "Behaviour stopped" );
    if (isEnabled()) {
        state = STATE_IDLE;
        updateTimer.stop();
        this->tcl->setForwardSpeed(0.0);
        this->tcl->setAngularSpeed(0.0);
        setEnabled(false);
        emit finished(this,false);
   }
}

void Behaviour_BallFollowing::reset()
{
    logger->debug( "Behaviour reset" );
    RobotBehaviour::reset();
    this->tcl->setForwardSpeed(0.0);
    this->tcl->setAngularSpeed(0.0);
}

QList<RobotModule*> Behaviour_BallFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append( cams );
    ret.append( compass );
    return ret;
}

QWidget* Behaviour_BallFollowing::createView(QWidget* parent)
{
    return new BallFollowingForm(parent, this);
}

void Behaviour_BallFollowing::testBehaviour( QString path )
{
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
        cvThreshold( gray, thresh, settings.value( "threshold", 100 ).toDouble(), 255, CV_THRESH_BINARY );
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
    cvThreshold( gray, thresh, settings.value( "threshold", 100 ).toDouble(), 255, CV_THRESH_BINARY );
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
        float robCenterX = this->getSettings().value("robCenterX").toFloat();
        float diff = robCenterX - x;
        float angleSpeed = 0.0;
        diff < 0.0 ? diff *= (-1) : diff;
        if(diff > this->getSettings().value("deltaBall").toFloat())
        {
            angleSpeed = this->getSettings().value("kpBall").toFloat() * ((robCenterX - x)/this->getSettings().value("maxDistance").toFloat());
        }
        tcl->setAngularSpeed(angleSpeed);
        tcl->setForwardSpeed(this->getSettings().value("fwSpeed").toFloat());

        data["ball_area"] = maxArea;
        data["ball_x"] = x;
        data["position_difference"] = diff;
        data["angular_speed"] = angleSpeed;
        data["forward_speed"] = this->getSettings().value("fwSpeed").toFloat();
        dataChanged( this );
        timerNoBall.start( 10000 );
    }
    else
    {
        tcl->setAngularSpeed( .0 );
        tcl->setForwardSpeed( .6 );
    }

    cvReleaseImage( &left );
    cvReleaseImage( &gray );
    cvReleaseImage( &thresh );
    cvReleaseImage( &hsv );
    cvReleaseImage( &left );
}

void Behaviour_BallFollowing::timerSlot()
{
    timerNoBall.stop();
    stop();
}
