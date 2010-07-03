#include "behaviour_ballfollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_BallFollowing/ballfollowingform.h>
#include <Behaviour_BallFollowing/blobs/blob.h>
#include <Behaviour_BallFollowing/blobs/BlobResult.h>
#include <opencv/highgui.h>

Behaviour_BallFollowing::Behaviour_BallFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Webcams *cams)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->cams = cams;
    connect(&updateTimer,SIGNAL(timeout()),this,SLOT(newData()));
    connect(&timerNoBall,SIGNAL(timeout()),this,SLOT(timerSlot()));

    setEnabled(false);
    state = STATE_IDLE;

}

bool Behaviour_BallFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_BallFollowing::start()
{
    this->setEnabled(true);
    updateTimer.start( 200 );
}

void Behaviour_BallFollowing::newData()
{
    if(this->isEnabled()) Behaviour_BallFollowing::ctrBallFollowing();
}

void Behaviour_BallFollowing::stop()
{
    if (isEnabled()) {
        updateTimer.stop();
       this->tcl->setForwardSpeed(0.0);
       this->tcl->setAngularSpeed(0.0);
       setEnabled(false);
       emit finished(this,false);
   }
}

void Behaviour_BallFollowing::reset()
{
    RobotBehaviour::reset();
    this->tcl->setForwardSpeed(0.0);
    this->tcl->setAngularSpeed(0.0);
}

QList<RobotModule*> Behaviour_BallFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
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
    // Get new frame.
    IplImage *left = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    IplImage *gray = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 1 );
    IplImage *thresh = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 1 );
    cams->grabLeft( left );

    // Apply threshold.
    cvCvtColor( left, gray, CV_RGB2GRAY );
    cvThreshold( gray, thresh, settings.value( "threshold", 100 ).toDouble(), 255, CV_THRESH_BINARY );

    // Do blob filtering.
    CBlobResult blobs( thresh, NULL, 0 );
    blobs.Filter( blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 200 );

    // Get largest blobs x position.
    float x = -1;
    int maxArea = 0;
    int maxBlob = -1;
    for ( int i = 0; i < blobs.GetNumBlobs(); i++ )
    {
        CBlob *blob = blobs.GetBlob( i );
        if ( blob->Area() > maxArea )
        {
            maxArea = blob->Area();
            maxBlob = i;
        }
    }
    if ( maxBlob > 0 )
    {
        x = blobs.GetBlob( maxBlob )->Moment( 1, 0 );
        blobs.GetBlob( maxBlob )->FillBlob( left, cvScalar( 255, 0, 0 ), 0, 0 );
    }

    emit printFrame( left );

    if ( x > 0 )
    {
        timerNoBall.stop();
        state = STATE_SEEN_BALL;
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
        timerNoBall.start(2000);
    }

    cvReleaseImage( &left );
    cvReleaseImage( &gray );
    cvReleaseImage( &thresh );
}

void Behaviour_BallFollowing::timerSlot()
{
    timerNoBall.stop();
    switch(this->state)
    {
    case STATE_SEEN_BALL:
        state = STATE_FORWARD;
        tcl->setAngularSpeed( .0 );
        timerNoBall.start(2000);
        break;
    case STATE_FORWARD:
        state = STATE_TURNING;
        tcl->setForwardSpeed( .0 );
        tcl->setAngularSpeed( .1 );
        timerNoBall.start(2000);
        break;
    case STATE_TURNING:
        state = STATE_FAILED;
        this->setHealthToSick("no ball in sight");
         emit finished(this,false);
        break;
    }

}
