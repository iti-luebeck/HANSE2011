#include "objecttracker.h"
#include <opencv/cv.h>
#include <Behaviour_BallFollowing/blobs/blob.h>
#include <Behaviour_BallFollowing/blobs/BlobResult.h>
#include <Framework/robotmodule.h>
#include <Framework/clahe.h>
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
    cvDilate(thresh, thresh, NULL, 20);
    cvErode(thresh, thresh, NULL, 20);

    CBlobResult blobs(thresh, NULL, 0);
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 100 );

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

void ObjectTracker::applyHomomorphic()
{
    int order = 0;
    int d = 10;

    IplImage *img = new IplImage(gray);
    cv::Mat A, H;
    IplImage* im_l, * im_f, * im_nf, * im_n, * im_e;

    int rows = img->height;
    int cols = img->width;

    im_l  = cvCreateImage(cvSize(cols,rows), 32, 1);
    im_f  = cvCreateImage(cvSize(cols,rows), 32, 1);
    im_nf = cvCreateImage(cvSize(cols,rows), 32, 1);
    im_n  = cvCreateImage(cvSize(cols,rows), 32, 1);
    im_e  = cvCreateImage(cvSize(cols,rows), 32, 1);

    A.create(rows,cols, CV_32F);
    H.create(rows,cols,CV_32F);

    // Filter mask for Butterworth high pass filter
    for(int i = 0; i < rows; i++) {
       for(int j = 0; j < cols; j++) {
            A.at<float>(i,j) =  sqrt(pow((i-(rows/2)),2)+ pow((j-(cols/2)),2));
            H.at<float>(i,j) =  1 /(1+(pow((d / A.at<float>(i,j)),(2*order))));
        }
    }

    // Threshholds:
    float alphaL = 0.999;
    float alphaH = 1.01;
    for(int i = 0; i < H.rows; i++) {
        for (int j = 0; j < H.cols; j++) {
            H.at<float>(i,j) = 1 - ((alphaH-alphaL)*H.at<float>(i,j))+alphaL;
        }
    }
    // scale image
    cvConvertScale(img, im_l,1/255.0f);

    // Add to every pixel value + 1
    for(int i = 0; i < im_l->height; i++) {
        for (int j = 0; j < im_l->width; j++) {
            CvScalar scalar = cvGet2D(im_l,i,j);
            scalar.val[0] = scalar.val[0] + 1;
            cvSet2D(im_l,i,j,scalar);
        }
    }

    // log of image
    cvLog(im_l, im_l);
    // DFT of logged image
    cvDFT(im_l, im_f, CV_DXT_SCALE);
    //Filter Applying DFT Image
    float tmp;
    for(int i = 0; i < im_f->height; i++) {
        for (int j = 0; j < im_f->width; j++) {
            CvScalar scalar = cvGet2D(im_f,i,j);
            tmp = (float) scalar.val[0];
            tmp = tmp * H.at<float>(i,j);
            scalar.val[0] = tmp;
            cvSet2D(im_nf,i,j,scalar);
        }
    }

    // Inverse DFT of filtered image
    cvDFT(im_nf,im_n,CV_DXT_INVERSE);
    cvAbs(im_n,im_n);
    // Inverse logarithmus
    cvExp(im_n,im_e);
    // Add to every pixel value + 1
    for(int i = 0; i < im_e->height; i++) {
        for (int j = 0; j < im_e->width; j++) {
            CvScalar scalar = cvGet2D(im_e,i,j);
            scalar.val[0] = scalar.val[0] - 1;
            cvSet2D(im_e,i,j,scalar);
        }
    }

    cvConvertScale(im_e, img, 255);

    cvReleaseImage(&im_l);
    cvReleaseImage(&im_f);
    cvReleaseImage(&im_nf);
    cvReleaseImage(&im_n);
    cvReleaseImage(&im_e);

}

void ObjectTracker::applyClahe()
{
    IplImage *ipl_gray = new IplImage(gray);
    cvCLAdaptEqualize(ipl_gray, ipl_gray, 10, 10, 255, 1.0f, CV_CLAHE_RANGE_FULL);
}

void ObjectTracker::findMovingObject()
{
    if (!lastGray.empty()) {
        Mat temp = gray - lastGray;
        temp = temp + (lastGray - gray);
//        temp = 0.5 * gray + temp;
//        for (int i = 0; i < temp.rows; i++) {
//            for (int j = 0; j < temp.cols; j++) {
//                int score = (int)temp.at<unsigned char>(i,j) + (int)gray.at<unsigned char>(i,j);
//                score /= 2;
//                temp.at<unsigned char>(i,j) = (unsigned char)score;
//            }
//        }

        gray.copyTo(lastGray);
        temp.copyTo(gray);
    } else {
        gray.copyTo(lastGray);
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
