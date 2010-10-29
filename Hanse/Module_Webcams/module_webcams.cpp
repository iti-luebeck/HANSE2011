#include "module_webcams.h"
#include <Module_Webcams/form_webcams.h>
#include <assert.h>
#include <opencv/highgui.h>

Module_Webcams::Module_Webcams( QString id ) :
        RobotModule_MT( id )
{
    VI.setUseCallback( true );
    leftConnected = false;
    rightConnected = false;
    bottomConnected = false;
    leftFramerate = 5;
    rightFramerate = 5;
    bottomFramerate = 5;
    nCams = 0;

    settingsChanged();

    leftFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    rightFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    bottomFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
}

Module_Webcams::~Module_Webcams()
{
    stopWebcams();
    cvReleaseImage( &leftFrame );
    cvReleaseImage( &rightFrame );
    cvReleaseImage( &bottomFrame );
}

void Module_Webcams::stopWebcams()
{
    if ( leftConnected ) VI.stopDevice( leftID );
    if ( rightConnected ) VI.stopDevice( rightID );
    if ( bottomConnected ) VI.stopDevice( bottomID );
}

void Module_Webcams::grabLeft( IplImage *left )
{
    if ( leftConnected )//&& VI.isFrameNew(leftID) )
    {
        mutex.lock();
        assert( left->width == WEBCAM_WIDTH && left->height == WEBCAM_HEIGHT );
        VI.getPixels( leftID, (unsigned char *)left->imageData, false, true );
        mutex.unlock();
    }
}

void Module_Webcams::grabRight( IplImage *right )
{
    if ( rightConnected )//&& VI.isFrameNew(rightID))
    {
        mutex.lock();
        assert( right->width == WEBCAM_WIDTH && right->height == WEBCAM_HEIGHT );
        VI.getPixels( rightID, (unsigned char *)right->imageData, false, true );
        mutex.unlock();
    }
}

void Module_Webcams::grabBottom( IplImage *bottom )
{
    if ( bottomConnected )//&& VI.isFrameNew(bottomID))
    {
        mutex.lock();
        assert( bottom->width == WEBCAM_WIDTH && bottom->height == WEBCAM_HEIGHT );
        VI.getPixels( bottomID, (unsigned char *)bottom->imageData, false, true );
        mutex.unlock();
    }
}

void Module_Webcams::grabBottom( cv::Mat &bottom )
{
    QTime blub;
    qDebug() << "cam thread id";
    qDebug() << QThread::currentThreadId();
    if ( bottomConnected )//&& VI.isFrameNew(bottomID))
    {
        mutex.lock();
        blub.restart();
        assert( bottom.cols == WEBCAM_WIDTH && bottom.rows == WEBCAM_HEIGHT );
        addData("run assert",blub.elapsed());
        blub.restart();
        VI.getPixels( bottomID, (unsigned char *)bottom.data, true, true );
        addData("run getPix",blub.elapsed());
        mutex.unlock();
    }
}
//
//void Module_Webcams::grabBottom(QImage bottom1)
//{
//    cv::Mat bottom;
//    bottom.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
//    qDebug() << "cam thread id";
//    qDebug() << QThread::currentThreadId();
//    if ( bottomConnected )//&& VI.isFrameNew(bottomID))
//    {
//        mutex.lock();
//        assert( bottom.cols == WEBCAM_WIDTH && bottom.rows == WEBCAM_HEIGHT );
//        VI.getPixels( bottomID, (unsigned char *)bottom.data, true, true );
//        mutex.unlock();
//    }
//     QImage image1((unsigned char*)bottom.data, bottom.cols, bottom.rows, QImage::Format_RGB888);
//     bottom1 = image1.copy();
//}

void Module_Webcams::reset()
{
    mutex.lock();
    stopWebcams();

    if ( this->isEnabled() )
    {
        if ( 0 <= bottomID && bottomID < nCams && bottomEnabled )
        {
            VI.setIdealFramerate( bottomID, bottomFramerate );
            bottomConnected = VI.setupDevice( bottomID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            bottomConnected = false;
        }
        if ( 0 <= leftID && leftID < nCams && leftEnabled )
        {
            VI.setIdealFramerate( leftID, leftFramerate );
            leftConnected = VI.setupDevice( leftID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            leftConnected = false;
        }
        if ( 0 <= rightID && rightID < nCams && rightEnabled )
        {
            VI.setIdealFramerate( rightID, rightFramerate );
            rightConnected = VI.setupDevice( rightID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            rightConnected = false;
        }
    }
    mutex.unlock();
}

void Module_Webcams::terminate()
{
    stopWebcams();
}




void Module_Webcams::statusChange( bool value )
{

}

void Module_Webcams::showSettings( int camNr )
{
    VI.showSettingsWindow( camNr );
}

void Module_Webcams::settingsChanged()
{
    nCams = VI.listDevices( true );
    leftID = this->getSettingsValue("leftID",0).toInt();
    rightID = this->getSettingsValue("rightID",1).toInt();
    bottomID = this->getSettingsValue("bottomID",2).toInt();
    leftEnabled = this->getSettingsValue("leftEnabled",true).toBool();
    rightEnabled = this->getSettingsValue("rightEnabled",true).toBool();
    bottomEnabled = this->getSettingsValue("bottomEnabled",true).toBool();
    leftFramerate = this->getSettingsValue("leftFramerate",5).toInt();
    rightFramerate = this->getSettingsValue("rightFramerate",5).toInt();
    bottomFramerate = this->getSettingsValue("bottomFramerate",5).toInt();

//    leftID = settings.value( "leftID", 0 ).toInt();
//    rightID = settings.value( "rightID", 1 ).toInt();
//    bottomID = settings.value( "bottomID", 2 ).toInt();
//    leftEnabled = settings.value("leftEnabled",true).toBool();
//    rightEnabled = settings.value("rightEnabled",true).toBool();
//    bottomEnabled = settings.value("bottomEnabled",true).toBool();
//    leftFramerate = settings.value("leftFramerate",5).toInt();
//    rightFramerate = settings.value("rightFramerate",5).toInt();
//    bottomFramerate = settings.value("bottomFramerate",5).toInt();
    if ( isEnabled() )
    {
        reset();
    }
}

QWidget *Module_Webcams::createView( QWidget *parent )
{
    Form_Webcams *cams = new Form_Webcams( this, parent );
    return cams;
}

QList<RobotModule *> Module_Webcams::getDependencies()
{
    return QList<RobotModule *>();
}
