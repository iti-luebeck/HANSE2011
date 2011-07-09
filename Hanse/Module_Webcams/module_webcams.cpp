#include "module_webcams.h"
#include <Module_Webcams/form_webcams.h>
#include <assert.h>

using namespace cv;

Module_Webcams::Module_Webcams( QString id ) :
        RobotModule( id )
{
    frontCap = NULL;
    frontID = 1;
    frontEnabled = false;

    bottomCap = NULL;
    bottomID = 2;
    bottomEnabled = false;

    updateTimer.moveToThread(this);
    QObject::connect(&updateTimer, SIGNAL(timeout()), this, SLOT(grabCams()));
}

Module_Webcams::~Module_Webcams()
{
    stopWebcams();
    if (frontFrame) cvReleaseImage(&frontFrame);
    if (bottomFrame) cvReleaseImage(&bottomFrame);
    if (frontCap) cvReleaseCapture(&frontCap);
    if (bottomCap) cvReleaseCapture(&bottomCap);
}

void Module_Webcams::init()
{
    dataLockerMutex.lock();
    nCams = this->numAvailableCams();

    frontFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    bottomFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    dataLockerMutex.unlock();

    this->setSettingsValue("frontEnabled", false);
    this->setSettingsValue("bottomEnabled", false);

    settingsChanged();
    updateTimer.start(250);
}

void Module_Webcams::stopWebcams()
{
    if ( frontCap != NULL ) {
        cvReleaseCapture(&frontCap);
        frontCap = NULL;
    }
    if ( bottomCap != NULL  ) {
        cvReleaseCapture(&bottomCap);
        bottomCap = NULL;
    }
}

void Module_Webcams::reset()
{
    this->updateTimer.stop();
    qDebug() << "cams reset";
    dataLockerMutex.lock();
    stopWebcams();
    this->nCams = this->numAvailableCams();
    int connectedCams = 0;

    if ( this->isEnabled() )
    {
        qDebug() << "gonna connect";
        if ( 0 <= bottomID && bottomEnabled && connectedCams < nCams)
        {
            logger->debug("Trying to Connect bottom Cam with ID "+QString::number(bottomID));
            bottomCap = cvCaptureFromCAM(bottomID);
            if (bottomCap) {
                connectedCams++;
                cvSetCaptureProperty(bottomCap, CV_CAP_PROP_FRAME_WIDTH, 640);
                cvSetCaptureProperty(bottomCap, CV_CAP_PROP_FRAME_HEIGHT, 480);
            } else {
                logger->debug("...fail");
                bottomCap = NULL;
            }
        }

        if ( 0 <= frontID && frontEnabled && connectedCams < nCams && (frontID != bottomID || !bottomEnabled) )
        {
            logger->debug("Trying to Connect front Cam with ID "+QString::number(frontID));
            frontCap = cvCaptureFromCAM(frontID);
            if (frontCap)
            {
                logger->debug("...ok");
                connectedCams++;
                cvSetCaptureProperty(frontCap,CV_CAP_PROP_FRAME_WIDTH,640);
                cvSetCaptureProperty(frontCap,CV_CAP_PROP_FRAME_HEIGHT,480);
            }
            else
            {
                logger->debug("...fail");
                frontCap = NULL;
            }

        }
    }
    dataLockerMutex.unlock();

    this->updateTimer.start(250);
}

std::vector<int> Module_Webcams::numOfCams()
{
    return this->camInd;
}

int Module_Webcams::numAvailableCams()
{
    this->camInd.clear();
    QString path = "/sys/class/video4linux/";
    QDir dir(path);
    dir.setNameFilters(QStringList("video*"));
    QStringList entries = dir.entryList();
    for(int i = 0; i < entries.size(); i++)
    {
        dir.cd(entries.at(i));

        QFile file(dir.path().append("/name"));
        file.open(QIODevice::ReadOnly);
        QTextStream stream(&file);
        QString camName = stream.readLine();
        if(camName != "WebCam")
        {
            QString id = entries.at(i);
            this->camInd.push_back(id.remove("video").toInt());
        }
        dir.cdUp();
    }
    return camInd.size();
}

void Module_Webcams::grabCams()
{
    if (!isEnabled())
        return;

    bool bottomSetToSick = false;
    dataLockerMutex.lock();
    if (bottomEnabled) {
        if (bottomCap != NULL) {
            IplImage *frame = cvQueryFrame(bottomCap);
            if (frame) {
                this->setHealthToOk();
                cvConvertImage(frame, bottomFrame, CV_CVTIMG_SWAP_RB);
                emit newBottomImage(Mat(bottomFrame));
            } else {
                this->setHealthToSick("bottom image is null (camera died in operation?)");
                bottomSetToSick = true;
            }
        } else {
            this->setHealthToSick("bottom cap is null (camera not connected?)");
            bottomSetToSick = true;
        }
    }
    if (frontEnabled) {
        if (frontCap != NULL) {
            IplImage *frame = cvQueryFrame(frontCap);
            if (frame) {
                if (!bottomSetToSick) {
                    this->setHealthToOk();
                }
                cvConvertImage(frame, frontFrame, CV_CVTIMG_SWAP_RB);
                emit newFrontImage(Mat(frontFrame));
            } else {
                this->setHealthToSick("front image is null (camera died in operation?)");
            }
        } else {
            this->setHealthToSick("front cap is null (camera not connected?)");
        }
    }
    dataLockerMutex.unlock();
}

void Module_Webcams::terminate() {
    stopWebcams();
    RobotModule::terminate();
}

void Module_Webcams::settingsChanged() {
    frontID = this->getSettingsValue("frontID",0).toInt();
    bottomID = this->getSettingsValue("bottomID",2).toInt();
    frontEnabled = this->getSettingsValue("frontEnabled",false).toBool();
    bottomEnabled = this->getSettingsValue("bottomEnabled",false).toBool();

    if ( isEnabled() ) {
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
