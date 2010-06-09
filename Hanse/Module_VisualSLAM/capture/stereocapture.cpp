#include "stereocapture.h"
#include <QDebug>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <time.h>
#include "../helpers.h"

StereoCapture::StereoCapture(int width, int height, int device1, int device2)
{
    // Initialize class variables.
    this->width = width;
    this->height = height;
    this->device1 = device1;
    this->device2 = device2;

    // Enable callback. This should improve synchronicity.
    VI.setUseCallback(true);

    // Set the framerate as high as we can (also for synchronicity reasons).
    VI.setIdealFramerate(device1, 60);
    VI.setIdealFramerate(device2, 60);

    // Set up the devices with the specified frame sizes.
    connected1 = VI.setupDevice(device1, width, height);
    connected2 = VI.setupDevice(device2, width, height);

    // Create frame images.
    frame1 = cvCreateImage( cvSize(width, height), IPL_DEPTH_8U, 3 );
    frame2 = cvCreateImage( cvSize(width, height), IPL_DEPTH_8U, 3 );

    // Load class features.
    CvFileStorage *fileStorage = cvOpenFileStorage( "classifier_features.xml", NULL, CV_STORAGE_READ );
    CvMat *goalFeatures = (CvMat *)cvReadByName( fileStorage, NULL, "goal" );
    if ( goalFeatures != NULL )
    {
        CvMat *goalFeaturesT = cvCreateMat( goalFeatures->cols, goalFeatures->rows, goalFeatures->type );
        cvTranspose( goalFeatures, goalFeaturesT );
        classFeatures.push_back( goalFeaturesT );
        classLabels.push_back( GOAL_LABEL );
    }
    CvMat *ballFeatures = (CvMat *)cvReadByName( fileStorage, NULL, "ball" );
    if ( ballFeatures != NULL )
    {
        CvMat *ballFeaturesT = cvCreateMat( ballFeatures->cols, ballFeatures->rows, ballFeatures->type );
        cvTranspose( ballFeatures, ballFeaturesT );
        classFeatures.push_back( ballFeaturesT );
        classLabels.push_back( BALL_LABEL );
    }

    QObject::connect( &feature1, SIGNAL( finished() ), SLOT( surfDone1() ) );
    QObject::connect( &feature2, SIGNAL( finished() ), SLOT( surfDone2() ) );
    done1 = false;
    done2 = false;

    // Calibrate the stereo cameras.
    initStereoCalibration();
}

StereoCapture::~StereoCapture()
{
    cvReleaseMat(&Q);
    cvReleaseMat(&mapX_left);
    cvReleaseMat(&mapY_left);
    cvReleaseMat(&mapX_right);
    cvReleaseMat(&mapY_right);
}

void StereoCapture::initStereoCalibration()
{
    // Initialize known extrinsic and intrinsic camara parameters.
    double cameraMatrix1Arr[] = { 489.2645,        0, 324.8205,
                                         0, 490.4222, 247.2733,
                                         0,        0,        1 };
    CvMat *cameraMatrix1 = cvCreateMat(3, 3, CV_64F);
    cvInitMatHeader(cameraMatrix1, 3, 3, CV_64F, cameraMatrix1Arr, CV_AUTOSTEP);

    double cameraMatrix2Arr[] = { 491.9357,        0, 327.8587,
                                         0, 493.2895, 240.4857,
                                         0,        0,        1 };
    CvMat *cameraMatrix2 = cvCreateMat(3, 3, CV_64F);
    cvInitMatHeader(cameraMatrix2, 3, 3, CV_64F, cameraMatrix2Arr, CV_AUTOSTEP);

    double distCoeffs1Arr[] = { 0.0705, -0.0718, -0.0009, -0.0015, 0 };
    CvMat *distCoeffs1 = cvCreateMat(1, 5, CV_64F);
    cvInitMatHeader(distCoeffs1, 1, 5, CV_64F, distCoeffs1Arr, CV_AUTOSTEP);

    double distCoeffs2Arr[] = { 0.0734, -0.1237, 0.0016, -0.0050, 0 };
    CvMat *distCoeffs2 = cvCreateMat(1, 5, CV_64F);
    cvInitMatHeader(distCoeffs2, 1, 5, CV_64F, distCoeffs2Arr, CV_AUTOSTEP);

    double RArr[] = { 0.9983, -0.0034, -0.0590,
                      0.0021,  0.9997, -0.0225,
                      0.0591,  0.0224,  0.9980 };
    CvMat *R = cvCreateMat(3, 3, CV_64F);
    cvInitMatHeader(R, 3, 3, CV_64F, RArr, CV_AUTOSTEP);

    double TArr[] = { -217.8341, 0.2694, -4.4763 };
    CvMat *T = cvCreateMat(3, 1, CV_64F);
    cvInitMatHeader(T, 3, 1, CV_64F, TArr, CV_AUTOSTEP);

    newT = -217.8802;

    // Calculate rectification matrices.
    CvMat *R1 = cvCreateMat(3, 3, CV_64F);
    CvMat *R2 = cvCreateMat(3, 3, CV_64F);
    CvMat *P1 = cvCreateMat(3, 4, CV_64F);
    CvMat *P2 = cvCreateMat(3, 4, CV_64F);
    Q = cvCreateMat( 4, 4, CV_64F );

    cvStereoRectify(cameraMatrix1,              // intrinsic parameters of the left camera
                    cameraMatrix2,              // intrinsic parameters of the right camera
                    distCoeffs1,                // distortion parameters of the left camera
                    distCoeffs2,                // distortion parameters of the right camera
                    cvSize(640, 480),           // image size
                    R,                          // rotation of right camera with respect to left camera
                    T,                          // translation of right camera with respect to left camera
                    R1,                         // rectification rotations
                    R2,
                    P1,                         // projection matrices into the new coordinate system
                    P2,
                    Q,                          // disparity to depth mapping
                    CV_CALIB_ZERO_DISPARITY,    // principal points should be the same
                    0);                         // zoom in so that there is no border

    // Calculate rectification mappings.
    f = 500;
    cm = 320;
    cn = 240;

    double newcameraMatrix_leftArr[] = { f, 0, cm,
                                         0, f, cn,
                                         0, 0,  1 };
    CvMat *newCameraMatrix_left = cvCreateMat(3, 3, CV_64F);
    cvInitMatHeader(newCameraMatrix_left, 3, 3, CV_64F, newcameraMatrix_leftArr, CV_AUTOSTEP);
    double newcameraMatrix_rightArr[] = { f, 0, cm,
                                         0, f, cn,
                                         0, 0,  1 };
    CvMat *newCameraMatrix_right = cvCreateMat(3, 3, CV_64F);
    cvInitMatHeader(newCameraMatrix_right, 3, 3, CV_64F, newcameraMatrix_rightArr, CV_AUTOSTEP);

    mapX_left = cvCreateMat(480, 640, CV_32FC1);
    mapY_left = cvCreateMat(480, 640, CV_32FC1);
    mapX_right = cvCreateMat(480, 640, CV_32FC1);
    mapY_right = cvCreateMat(480, 640, CV_32FC1);
    cvInitUndistortRectifyMap(cameraMatrix1, distCoeffs1, R1, newCameraMatrix_left, mapX_left, mapY_left);
    cvInitUndistortRectifyMap(cameraMatrix2, distCoeffs2, R2, newCameraMatrix_right, mapX_right, mapY_right);

    cvReleaseMat(&newCameraMatrix_left);
    cvReleaseMat(&newCameraMatrix_right);
    cvReleaseMat(&cameraMatrix1);
    cvReleaseMat(&cameraMatrix2);
    cvReleaseMat(&distCoeffs1);
    cvReleaseMat(&distCoeffs2);
    cvReleaseMat(&R);
    cvReleaseMat(&T);
    cvReleaseMat(&R1);
    cvReleaseMat(&R2);
    cvReleaseMat(&P1);
    cvReleaseMat(&P2);
}

void StereoCapture::grab( vector<CvMat *> *descriptors, vector<CvScalar> *pos2D, vector<CvScalar> *pos3D, vector<int> *classesVector)
{
    this->descriptors = descriptors;
    this->pos2D = pos2D;
    this->pos3D = pos3D;
    this->classesVector = classesVector;

//    start = clock();
    if (connected1) VI.getPixels(device1, (unsigned char *)frame1->imageData, true, true);
    if (connected2) VI.getPixels(device2, (unsigned char *)frame2->imageData, true, true);
//    stop = clock();
//    qDebug( "WEBCAM %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

//    vector<CvScalar> calonderKeypoints1;
//    vector<CvScalar> calonderKeypoints2;
//    feature.matchFeatures( frame1, frame2, calonderKeypoints1, calonderKeypoints2 );

//    start = clock();
    keypoints1.clear();
    feature1.findFeatures( frame1, keypoints1 );

    keypoints2.clear();
    feature2.findFeatures( frame2, keypoints2 );
}

void StereoCapture::doCalculations()
{
    CvMat *descriptors1 = feature1.getDescriptor();
    CvMat *descriptors2 = feature2.getDescriptor();

    pos2D->clear();
    pos3D->clear();
    descriptors->clear();
    classesVector->clear();

    for ( int i = 0; i < (int)keypoints1.size(); i++ )
    {
        CvScalar point = keypoints1[i];
        cvCircle( frame1, cvPoint(point.val[0], point.val[1]), 20, cvScalar(255, 0, 0), 2, CV_FILLED);
        cvCircle( frame1, cvPoint(point.val[0], point.val[1]), 4, cvScalar(255, 0, 0), 4, CV_FILLED);
        // qDebug("%f %f", point->pt.x, point->pt.y);
    }

    // Check for occurences of the class features.
    int *classArray = new int[(int)keypoints1.size()];
    memset( classArray, 0, (int)keypoints1.size()*sizeof(int) );
    for ( int i = 0; i < (int)classFeatures.size(); i++ )
    {
        vector<CvPoint> classMatches;
        double xmin = 1000;
        double xmax = -1;
        double ymin = 1000;
        double ymax = -1;
        feature1.matchFeatures(descriptors1, classFeatures[i], classMatches);
        for ( int j = 0; j < (int)classMatches.size(); j++ )
        {
            int idx = classMatches[j].x;
            classArray[idx] = classLabels[i];

            if ( keypoints1[idx].val[0] < xmin )
            {
                xmin = keypoints1[idx].val[0];
            }
            if ( keypoints1[idx].val[0] > xmax )
            {
                xmax = keypoints1[idx].val[0];
            }
            if ( keypoints1[idx].val[1] < ymin )
            {
                ymin = keypoints1[idx].val[1];
            }
            if ( keypoints1[idx].val[1] < ymax )
            {
                ymax = keypoints1[idx].val[1];
            }
        }

        if ( (int)classMatches.size() > 3 )
        {
            classRects[i] = QRectF( xmin, ymin, xmax - xmin, ymax - ymin );
            classLastSeen[i] = QDateTime::currentDateTime();
        }
    }
    for ( int i = 0; i < (int)keypoints1.size(); i++ )
    {
        classesVector->push_back( classArray[i] );
    }

    for (int i = 0; i < (int)keypoints2.size(); i++)
    {
        CvScalar point = keypoints2[i];
        cvCircle( frame2, cvPoint(point.val[0], point.val[1]), 20, cvScalar(255, 0, 0), 2, CV_FILLED);
        cvCircle( frame2, cvPoint(point.val[0], point.val[1]), 4, cvScalar(255, 0, 0), 4, CV_FILLED);
        // qDebug("%f %f", point->pt.x, point->pt.y);
    }
//    stop = clock();
//    qDebug( "FEATURES %0.3f msec (%d, %d)", (double)(1000 * (stop - start) / CLOCKS_PER_SEC),
//            keypoints1.size(), keypoints2.size() );

//    start = clock();
    vector<CvPoint> matches;
    feature1.matchFeatures(descriptors1, descriptors2, matches);
    feature1.updateThreshold( keypoints1.size() );
    feature2.updateThreshold( keypoints2.size() );
//    stop = clock();
//    qDebug( "MATCH %0.3f msec (%d pairs)", (double)(1000 * (stop - start) / CLOCKS_PER_SEC), (int)matches.size() );

//    start = clock();
    if ((int)matches.size() > 0)
    {
        CvMat *xl = cvCreateMat( 2, matches.size(), CV_32F );
        CvMat *xr = cvCreateMat( 2, matches.size(), CV_32F );
        for ( int i = 0; i < (int)matches.size(); i++ )
        {
            int matchix = matches[i].x;
            int matchiy = matches[i].y;

            CvScalar point = keypoints1[matchix];
            cvCircle( frame1, cvPoint(point.val[0], point.val[1]), 20, cvScalar(0, 255, 0), 4, CV_FILLED);
            cvCircle( frame1, cvPoint(point.val[0], point.val[1]), 4, cvScalar(0, 255, 0), 6, CV_FILLED);

            point = keypoints2[matchiy];
            cvCircle( frame2, cvPoint(point.val[0], point.val[1]), 20, cvScalar(0, 255, 0), 4, CV_FILLED);
            cvCircle( frame2, cvPoint(point.val[0], point.val[1]), 4, cvScalar(0, 255, 0), 6, CV_FILLED);

            CvMat *feature = cvCreateMat( 64, 1, CV_32F );
            for ( int j = 0; j < 64; j++ )
            {
                cvmSet( feature, j, 0, cvmGet( descriptors1, matchix, j ) );
            }
            descriptors->push_back( feature );

            // Map is transposed !!!
            point = keypoints1[matchix];
            cvmSet( xl, 0, i, cvmGet(mapX_left, cvRound(point.val[1]), cvRound(point.val[0])) );
            cvmSet( xl, 1, i, cvmGet(mapY_left, cvRound(point.val[1]), cvRound(point.val[0])) );

            point = keypoints2[matchiy];
            cvmSet( xr, 0, i, cvmGet(mapX_right, cvRound(point.val[1]), cvRound(point.val[0])) );
            cvmSet( xr, 1, i, cvmGet(mapY_right, cvRound(point.val[1]), cvRound(point.val[0])) );
        }

        stereoTriangulation( xl, xr, *pos3D, this->newT );

        if ( (int)pos3D->size() == 0 )
        {
            descriptors->clear();
            classesVector->clear();
        }

        for ( int i = (int)pos3D->size() - 1; i >= 0; i-- )
        {
            if ( (*pos3D)[i].val[2] > 10 || (*pos3D)[i].val[2] < 0.5 )
            {
                pos3D->erase( pos3D->begin() + i );
                cvReleaseMat( &(*descriptors)[i] );
                descriptors->erase( descriptors->begin() + i );
                classesVector->erase( classesVector->begin() + i );
            }
        }
    }
//    stop = clock();
//    qDebug( "COPY & TRIANGULATION %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

//    cvReleaseMat( &descriptors1 );
//    descriptors1 = 0;
//    cvReleaseMat( &descriptors2 );
//    descriptors2 = 0;

    done();
}

void StereoCapture::normalizePixels(CvMat *x, CvMat *xn, double f, double cm, double cn)
{
    for ( int i = 0; i < x->cols; i++ )
    {
        cvmSet( xn, 0, i, (cvmGet(x, 0, i) - cm) / f );
        cvmSet( xn, 1, i, (cvmGet(x, 1, i) - cn) / f );
        cvmSet( xn, 2, i, 1 );
    }
}

void StereoCapture::stereoTriangulation(CvMat *xl, CvMat *xr, vector<CvScalar> &X, double T)
{
    // Normalize pixels. Normalized pixels will be in homogeneous coordinates.
    CvMat *xln = cvCreateMat( 3, xl->cols, CV_32F );
    CvMat *xrn = cvCreateMat( 3, xr->cols, CV_32F );
    normalizePixels( xl, xln, this->f, this->cm, this->cn );
    normalizePixels( xr, xrn, this->f, this->cm, this->cn );

    // Do the triangulation.
    double n_ll = 0.0, n_rr = 0.0, n_lT = 0.0, n_rT = 0.0, n_rl = 0.0;
    double DD = 0.0, NN1 = 0.0, NN2 = 0.0;
    double Z1 = 0.0, Z2 = 0.0;
    double X11 = 0.0, X12 = 0.0, X13 = 0.0, X21 = 0.0, X22 = 0.0, X23 = 0.0;
    int nGreaterZero = 0;
    int nSmallerZero = 0;

    for ( int i = 0; i < xl->cols; i++ )
    {
        n_ll = cvmGet(xln, 0, i)*cvmGet(xln, 0, i) + cvmGet(xln, 1, i)*cvmGet(xln, 1, i) + cvmGet(xln, 2, i)*cvmGet(xln, 2, i);
        n_rr = cvmGet(xrn, 0, i)*cvmGet(xrn, 0, i) + cvmGet(xrn, 1, i)*cvmGet(xrn, 1, i) + cvmGet(xrn, 2, i)*cvmGet(xrn, 2, i);
        n_lT = cvmGet(xln, 0, i) * T; //cvmGet(xln, 0, i)*cvmGet(T, 0, 0) + cvmGet(xln, 1, i)*cvmGet(T, 1, 0) + cvmGet(xln, 2, i)*cvmGet(T, 2, 0);
        n_rT = cvmGet(xrn, 0, i) * T; //cvmGet(xrn, 0, i)*cvmGet(T, 0, 0) + cvmGet(xrn, 1, i)*cvmGet(T, 1, 0) + cvmGet(xrn, 2, i)*cvmGet(T, 2, 0);
        n_rl = cvmGet(xln, 0, i)*cvmGet(xrn, 0, i) + cvmGet(xln, 1, i)*cvmGet(xrn, 1, i) + cvmGet(xln, 2, i)*cvmGet(xrn, 2, i);

        DD = n_ll*n_rr - n_rl*n_rl;
        NN1 = n_rl*n_rT - n_rr*n_lT;
        NN2 = n_ll*n_rT - n_lT*n_rl;

        Z1 = NN1 / DD;
        Z2 = NN2 / DD;

        X11 = cvmGet(xln, 0, i) * Z1;
        X12 = cvmGet(xln, 1, i) * Z1;
        X13 = cvmGet(xln, 2, i) * Z1;
        X21 = cvmGet(xrn, 0, i) * (Z2 - T); //cvmGet(xrn, 0, i) * (Z2 - cvmGet(T, 0, 0));
        X22 = cvmGet(xrn, 1, i) * Z2; //cvmGet(xrn, 1, i) * (Z2 - cvmGet(T, 1, 0));
        X23 = cvmGet(xrn, 2, i) * Z2; //cvmGet(xrn, 2, i) * (Z2 - cvmGet(T, 2, 0));

        CvScalar pos = cvScalar( 0.5 * (X11 + X21) / 1000,
                                 0.5 * (X12 + X22) / 1000,
                                 0.5 * (X13 + X23) / 1000 );
        if ( X13 + X23 > 0 )
        {
            nGreaterZero++;
        }
        else
        {
            nSmallerZero++;
        }

        X.push_back(pos);
    }

    // Check if left and right were switched.
    if ( (nGreaterZero + nSmallerZero) > 20 && nSmallerZero > nGreaterZero )
    {
        int temp = device1;
        device1 = device2;
        device2 = temp;
        X.clear();
    }

    cvReleaseMat( &xln );
    cvReleaseMat( &xrn );
}

bool StereoCapture::isConnected(int device)
{
    switch (device)
    {
    case 0:
        return connected1;
    case 1:
        return connected2;
    default:
        return false;
    }
}

IplImage *StereoCapture::getFrame( int cam )
{
    if ( cam == 0 )
    {
        return frame1;
    }
    else
    {
        return frame2;
    }
}

void StereoCapture::getObjectPosition( int classNr, QRectF &boundingBox, QDateTime &lastSeen )
{
    for ( int i = 0; i < (int)classLabels.size(); i++ )
    {
        if ( classLabels[i] == classNr )
        {
            boundingBox = classRects[i];
            lastSeen = classLastSeen[i];
            break;
        }
    }
}

void StereoCapture::surfDone1()
{
    if ( done2 )
    {
        done1 = false;
        done2 = false;
        doCalculations();
    }
    else
    {
        done1 = true;
    }
}

void StereoCapture::surfDone2()
{
    if ( done1 )
    {
        done1 = false;
        done2 = false;
        doCalculations();
    }
    else
    {
        done2 = true;
    }
}
