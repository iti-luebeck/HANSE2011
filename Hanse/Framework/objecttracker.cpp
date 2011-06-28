#include "objecttracker.h"
#include <opencv/cv.h>
#include <Behaviour_BallFollowing/blobs/blob.h>
#include <Behaviour_BallFollowing/blobs/BlobResult.h>
#include <Framework/robotmodule.h>
#include <vector>

using namespace cv;

ObjectTracker::ObjectTracker(RobotModule *parent)
{
    this->parent = parent;
    init();
}

void ObjectTracker::init()
{
    reset();
    loadFromSettings();
}

void ObjectTracker::reset()
{
    state = STATE_NOT_SEEN_YET;

    area = 0;
    meanX = 0;
    meanY = 0;
    orientation = 0;

    lastArea = 0;
    lastMeanX = 0;
    lastMeanY = 0;
    lastOrientation = 0;
}

void ObjectTracker::update(cv::Mat &frame)
{
    lastArea = area;
    lastMeanX = meanX;
    lastMeanY = meanY;
    lastOrientation = orientation;

    frame.copyTo(this->frame);
    getThresholdChannel();
}

void ObjectTracker::getThresholdChannel()
{
    vector<Mat> colors;
    gray.create(frame.rows, frame.cols, CV_8UC1);

    if (colorSpace == "rgb") {
        split(frame, colors);
        colors[channel % 3].copyTo(gray);
    } else {
        Mat hsvFrame(frame.size(), frame.type());
        cvtColor(frame, hsvFrame, CV_RGB2HSV);
        split(hsvFrame, colors);
        colors[channel % 3].copyTo(gray);
    }
}

double ObjectTracker::applyThreshold()
{
    binary.create(gray.size(), gray.type());
    if (inverted) {
        if (automatic) {
            return threshold(gray, binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
        } else {
            return threshold(gray, binary, thres, 255, THRESH_BINARY_INV);
        }
    } else {
        if (automatic) {
            return threshold(gray, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
        } else {
            return threshold(gray, binary, thres, 255, THRESH_BINARY);
        }
    }
}

void ObjectTracker::extractLargestBlob()
{
    IplImage *thresh = new IplImage(binary);

    CBlobResult blobs(thresh, NULL, 0);
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 500 );

    // Get largest blob.
    int maxArea = 0;
    int maxBlob = -1;
    for (int j = 0; j < blobs.GetNumBlobs(); j++) {
        CBlob *blob = blobs.GetBlob(j);
        if (blob->Area() > maxArea) {
            maxArea = blob->Area();
            maxBlob = j;
        }
    }

    if (maxBlob >= 0) {
        area = blobs.GetBlob(maxBlob)->Moment(0, 0);

        // First order moments -> mean position
        double m10 = blobs.GetBlob(maxBlob)->Moment(1, 0) / area;
        double m01 = blobs.GetBlob(maxBlob)->Moment(0, 1) / area;
        meanX = m10;
        meanY = m01;

        // Second order moments -> orientation
        double mu11 = blobs.GetBlob(maxBlob)->Moment(1, 1) / area;
        double mu20 = blobs.GetBlob(maxBlob)->Moment(2, 0) / area;
        double mu02 = blobs.GetBlob(maxBlob)->Moment(0, 2) / area;
        orientation = 0.5 * atan2( 2 * mu11 , ( mu20 - mu02 ) );

        // Draw blob to gray image.
        binary = binary.setTo(Scalar(0));
        blobs.GetBlob(maxBlob)->FillBlob(thresh, cvScalar(255), 0, 0);
    } else {
        area = 0;
    }
}

void ObjectTracker::estimateMoments()
{
    IplImage *ipl = new IplImage(binary);
    CvMoments M;
    cvMoments(ipl, &M, 1);

    // Get number of pipe pixels.
    area = cvGetSpatialMoment(&M, 0, 0);

    // First order moments -> mean position
    double m10 = cvGetSpatialMoment( &M, 1, 0 ) / area;
    double m01 = cvGetSpatialMoment( &M, 0, 1 ) / area;
    meanX = m10;
    meanY = m01;

    // Second order moments -> orientation
    double mu11 = cvGetCentralMoment( &M, 1, 1 ) / area;
    double mu20 = cvGetCentralMoment( &M, 2, 0 ) / area;
    double mu02 = cvGetCentralMoment( &M, 0, 2 ) / area;
    orientation = 0.5 * atan2( 2 * mu11 , ( mu20 - mu02 ) );

    // Theta is now the rotation angle reletive to the x axis.
    // We want it relative to the y axis -> 90° ccw
    orientation -= CV_PI / 2;
    if (orientation < -CV_PI) {
        orientation += CV_PI;
    }

    if (orientation < -CV_PI/2) {
        orientation += CV_PI;
    } else if (orientation > CV_PI/2) {
        orientation -= CV_PI;
    }

}

double ObjectTracker::getArea()
{
    return area;
}

double ObjectTracker::getMeanX()
{
    return meanX;
}

double ObjectTracker::getMeanY()
{
    return meanY;
}

double ObjectTracker::getOrientation()
{
    return orientation;
}

QString ObjectTracker::getState()
{
    return state;
}

QString ObjectTracker::getColorSpace()
{
    return colorSpace;
}

void ObjectTracker::setColorSpace(QString colorSpace)
{
    this->colorSpace = colorSpace;
    updateSettings();
}

int ObjectTracker::getChannel()
{
    return channel;
}

void ObjectTracker::setChannel(int channel)
{
    this->channel = channel;
    updateSettings();
}

bool ObjectTracker::isAutomaticThreshold()
{
    return automatic;
}

void ObjectTracker::setAutomaticThreshold(bool automatic)
{
    this->automatic = automatic;
    updateSettings();
}

double ObjectTracker::getThreshold()
{
    return thres;
}

void ObjectTracker::setThreshold(double thres)
{
    this->thres = thres;
    updateSettings();
}

bool ObjectTracker::isInverted()
{
    return this->inverted;
}

void ObjectTracker::setInverted(bool inverted)
{
    this->inverted = inverted;
    updateSettings();
}

void ObjectTracker::loadFromSettings()
{
    colorSpace = parent->getSettingsValue("color space", "rgb").toString();
    channel = parent->getSettingsValue("channel", 0).toInt();
    automatic = parent->getSettingsValue("automatic threshold", true).toBool();
    inverted = parent->getSettingsValue("inverted threshold", false).toBool();
    thres = parent->getSettingsValue("threshold", 0).toDouble();
}

void ObjectTracker::updateSettings()
{
    parent->setSettingsValue("color space", colorSpace);
    parent->setSettingsValue("channel", channel);
    parent->setSettingsValue("automatic threshold", automatic);
    parent->setSettingsValue("inverted threshold", inverted);
    parent->setSettingsValue("threshold", thres);
}

void ObjectTracker::grabFrame(cv::Mat &frame)
{
    this->frame.copyTo(frame);
}

void ObjectTracker::grabGray(cv::Mat &gray)
{
    std::vector<Mat> grays;
    grays.push_back(this->gray);
    grays.push_back(this->gray);
    grays.push_back(this->gray);
    merge(grays, gray);
}

void ObjectTracker::grabBinary(cv::Mat &binary)
{
    std::vector<Mat> binaries;
    binaries.push_back(this->binary);
    binaries.push_back(this->binary);
    binaries.push_back(this->binary);
    merge(binaries, binary);
}
