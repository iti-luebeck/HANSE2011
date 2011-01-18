#include "module_webcams.h"
#include <Module_Webcams/form_webcams.h>
#include <assert.h>

Module_Webcams::Module_Webcams( QString id ) :
        RobotModule_MT( id )
{
//    VI.setUseCallback( true );
    leftConnected = false;
    rightConnected = false;
    bottomConnected = false;
    leftFramerate = 5;
    rightFramerate = 5;
    bottomFramerate = 5;
    nCams = 3;

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
    if ( leftConnected ) leftCap.release();//cvReleaseCapture(&leftCam);//VI.stopDevice( leftID );
    if ( rightConnected ) rightCap.release();//cvReleaseCapture(&rightCam);//VI.stopDevice( rightID );
    if ( bottomConnected ) bottomCap.release();//cvReleaseCapture(&bottomCam);//VI.stopDevice( bottomID );
}

void Module_Webcams::grabLeft( IplImage *left )
{
    if ( leftConnected )//&& VI.isFrameNew(leftID) )
    {
        mutex.lock();
        assert( left->width == WEBCAM_WIDTH && left->height == WEBCAM_HEIGHT );
        if(leftCap.grab())
        {
            cv::Mat mat;
            leftCap.retrieve(mat,0);
            cv::imshow("blub",mat);
            left = new IplImage(mat.clone());
        }

//        cvGrabFrame(leftCam);
//        left = cvCloneImage(cvRetrieveFrame(leftCam));
//        VI.getPixels( leftID, (unsigned char *)left->imageData, false, true );
        mutex.unlock();
    }
}

void Module_Webcams::grabLeft( cv::Mat &left )
{
    if ( leftConnected )//&& VI.isFrameNew(bottomID))
    {
        mutex.lock();
        assert( left.cols == WEBCAM_WIDTH && left.rows == WEBCAM_HEIGHT );
        if(leftCap.grab())
            leftCap.retrieve(left);
        mutex.unlock();
    }
}

void Module_Webcams::grabRight( IplImage *right )
{
    if ( rightConnected )//&& VI.isFrameNew(rightID))
    {
        mutex.lock();
        assert( right->width == WEBCAM_WIDTH && right->height == WEBCAM_HEIGHT );
        if(rightCap.grab())
        {
            cv::Mat mat;
            rightCap.retrieve(mat);
            right = new IplImage(mat);
        }

//        cvGrabFrame(rightCam);
//        right = cvCloneImage(cvRetrieveFrame(rightCam));
//        VI.getPixels( rightID, (unsigned char *)right->imageData, false, true );
        mutex.unlock();
    }
}

void Module_Webcams::grabRight( cv::Mat &right )
{
    if ( rightConnected )//&& VI.isFrameNew(bottomID))
    {
        mutex.lock();
        assert( right.cols == WEBCAM_WIDTH && right.rows == WEBCAM_HEIGHT );
        if(rightCap.grab())
            rightCap.retrieve(right);
        mutex.unlock();
    }
}

void Module_Webcams::grabBottom( IplImage *bottom )
{
    if ( bottomConnected )//&& VI.isFrameNew(bottomID))
    {
        mutex.lock();
        assert( bottom->width == WEBCAM_WIDTH && bottom->height == WEBCAM_HEIGHT );
        if(bottomCap.grab())
        {
            cv::Mat mat;
            bottomCap.retrieve(mat);
            bottom = new IplImage(mat);
        }
        //        cvGrabFrame(bottomCam);
//        bottom = cvCloneImage(cvRetrieveFrame(bottomCam));
//        VI.getPixels( bottomID, (unsigned char *)bottom->imageData, false, true );
        mutex.unlock();
    }
}

void Module_Webcams::grabBottom( cv::Mat &bottom )
{
//    qDebug() << "cam thread id";
//    qDebug() << QThread::currentThreadId();
    if ( bottomConnected )//&& VI.isFrameNew(bottomID))
    {
        mutex.lock();
        assert( bottom.cols == WEBCAM_WIDTH && bottom.rows == WEBCAM_HEIGHT );
        if(bottomCap.grab())
            bottomCap.retrieve(bottom);

//        cvGrabFrame(bottomCam);
//        IplImage *img = (cvCloneImage(cvRetrieveFrame(bottomCam)));
//        cv::Mat mtx (img);
//        bottom = mtx.clone();
//        VI.getPixels( bottomID, (unsigned char *)bottom.data, true, true );
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
            bottomConnected = bottomCap.open(bottomID);
//            bottomCam = cvCaptureFromCAM(bottomID);
//            bottomConnected = true;
//            VI.setIdealFramerate( bottomID, bottomFramerate );
//            bottomConnected = VI.setupDevice( bottomID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            bottomConnected = false;
        }
        if ( 0 <= leftID && leftID < nCams && leftEnabled )
        {
            leftConnected = leftCap.open(leftID);
            //            VI.setIdealFramerate( leftID, leftFramerate );
//            leftConnected = VI.setupDevice( leftID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            leftConnected = false;
        }
        if ( 0 <= rightID && rightID < nCams && rightEnabled )
        {
            rightConnected = rightCap.open(rightID);
//            VI.setIdealFramerate( rightID, rightFramerate );
//            rightConnected = VI.setupDevice( rightID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            rightConnected = false;
        }
    }
    mutex.unlock();
    qDebug() << "left" << leftConnected;
        qDebug() << "right" << rightConnected;
            qDebug() << "bottom" << bottomConnected;
}

void Module_Webcams::terminate()
{
    stopWebcams();
    RobotModule_MT::terminate();
}




void Module_Webcams::statusChange( bool value )
{

}

void Module_Webcams::showSettings( int camNr )
{
//    VI.showSettingsWindow( camNr );
}

void Module_Webcams::settingsChanged()
{
//    nCams = VI.listDevices( true );
    leftID = this->getSettingsValue("leftID",0).toInt();
    rightID = this->getSettingsValue("rightID",1).toInt();
    bottomID = this->getSettingsValue("bottomID",2).toInt();
    leftEnabled = this->getSettingsValue("leftEnabled",true).toBool();
    rightEnabled = this->getSettingsValue("rightEnabled",true).toBool();
    bottomEnabled = this->getSettingsValue("bottomEnabled",true).toBool();
    leftFramerate = this->getSettingsValue("leftFramerate",5).toInt();
    rightFramerate = this->getSettingsValue("rightFramerate",5).toInt();
    bottomFramerate = this->getSettingsValue("bottomFramerate",5).toInt();

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
