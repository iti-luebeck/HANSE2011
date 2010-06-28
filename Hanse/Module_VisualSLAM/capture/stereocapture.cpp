#include "stereocapture.h"
#include <QDebug>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <time.h>
#include "../helpers.h"

StereoCapture::StereoCapture( int width, int height, int device1, int device2 )
{
    // Initialize class variables.
    this->width = width;
    this->height = height;
    this->device1 = device1;
    this->device2 = device2;

    Q = NULL;
    mapX_left = NULL;
    mapY_left = NULL;
    mapX_right = NULL;
    mapY_right = NULL;
    descriptors = NULL;
    pos = NULL;
    classes = NULL;
    frame1 = NULL;
    frame2 = NULL;
    connected1 = false;
    connected2 = false;
    feature1 = NULL;
    feature2 = NULL;
    count = 0;
}

StereoCapture::~StereoCapture()
{
    clear();
}

void StereoCapture::clear()
{
    if ( Q )
    {
        cvReleaseMat( &Q );
        Q = NULL;
    }
    if ( mapX_left )
    {
        cvReleaseMat(&mapX_left);
        mapX_left = NULL;
    }
    if ( mapY_left )
    {
        cvReleaseMat(&mapY_left);
        mapY_left = NULL;
    }
    if ( mapX_right )
    {
        cvReleaseMat(&mapX_right);
        mapX_right = NULL;
    }
    if ( mapY_right )
    {
        cvReleaseMat(&mapY_right);
        mapY_right = NULL;
    }

    keypoints1.clear();
    keypoints2.clear();
    if ( frame1 )
    {
        cvReleaseImage( &frame1 );
        frame1 = NULL;
    }
    if ( frame2 )
    {
        cvReleaseImage( &frame2 );
        frame2 = NULL;
    }
    if ( descriptors )
    {
        for ( int i = 0; i < (int)descriptors->size(); i++ )
        {
            cvReleaseMat( &descriptors->at(i) );
        }
        delete( descriptors );
        descriptors = NULL;
    }
    if ( pos )
    {
        delete( pos );
        pos = NULL;
    }
    if ( classes )
    {
        delete( classes );
        classes = NULL;
    }

    for ( int i = 0; i < (int)classFeatures.size(); i++ )
    {
        cvReleaseMat( &classFeatures[i] );
    }
    classFeatures.clear();
    classLabels.clear();
    classRects.clear();
    classLastSeen.clear();

    if ( connected1 )
    {
        VI.stopDevice( device1 );
        connected1 = false;
    }
    if ( connected2 )
    {
        VI.stopDevice( device2 );
        connected2 = false;
    }

    if ( feature1 )
    {
        delete( feature1 );
        feature1 = NULL;
    }
    if ( feature2 )
    {
        delete( feature2 );
        feature2 = NULL;
    }

    count = 0;
}

void StereoCapture::init( int device1, int device2 )
{
    clear();

    this->device1 = device1;
    this->device2 = device2;

    int numDevices = VI.listDevices( true );

    // Enable callback. This should improve synchronicity.
    VI.setUseCallback(true);

    if ( device1 < numDevices )
    {
        // Set the framerate as high as we can (also for synchronicity reasons).
        VI.setIdealFramerate(device1, 60);

        // Set up the devices with the specified frame sizes.
        connected1 = VI.setupDevice(device1, width, height);
    }
    else
    {
        connected1 = false;
    }

    if ( device2 < numDevices )
    {
        // Set the framerate as high as we can (also for synchronicity reasons).
        VI.setIdealFramerate(device2, 60);

        // Set up the devices with the specified frame sizes.
        connected2 = VI.setupDevice(device2, width, height);
    }
    else
    {
        connected2 = false;
    }

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
        classRects.push_back( QRectF() );
        classLastSeen.push_back( QDateTime() );
    }
    CvMat *ballFeatures = (CvMat *)cvReadByName( fileStorage, NULL, "ball" );
    if ( ballFeatures != NULL )
    {
        CvMat *ballFeaturesT = cvCreateMat( ballFeatures->cols, ballFeatures->rows, ballFeatures->type );
        cvTranspose( ballFeatures, ballFeaturesT );
        classFeatures.push_back( ballFeaturesT );
        classLabels.push_back( BALL_LABEL );
        classRects.push_back( QRectF() );
        classLastSeen.push_back( QDateTime() );
    }
    cvReleaseFileStorage( &fileStorage );

    done1 = false;
    done2 = false;

    descriptors = new vector<CvMat *>;
    pos = new vector<CvScalar>;
    classes = new vector<int>;

    feature1 = new Feature();
    feature2 = new Feature();
    QObject::connect( feature1, SIGNAL( finished() ), SLOT( surfDone1() ) );
    QObject::connect( feature2, SIGNAL( finished() ), SLOT( surfDone2() ) );

    // Calibrate the stereo cameras.
    initStereoCalibration();
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
    cvInitUndistortRectifyMap(cameraMatrix1, distCoeffs1, R1, newCameraMatrix_left, mapX_right, mapY_right);
    cvInitUndistortRectifyMap(cameraMatrix2, distCoeffs2, R2, newCameraMatrix_right, mapX_left, mapY_left);

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

void StereoCapture::grab( bool saveImages )
{
    captureMutex.lock();

    if (connected1) VI.getPixels(device1, (unsigned char *)frame1->imageData, true, true);
    if (connected2) VI.getPixels(device2, (unsigned char *)frame2->imageData, true, true);

    if ( saveImages )
    {
        char leftName[100];
        sprintf( leftName, "capture/left%04d.jpg", count );
        cvSaveImage( leftName, frame1 );
        char rightName[100];
        sprintf( rightName, "capture/right%04d.jpg", count );
        cvSaveImage( rightName, frame2 );
    }

    count++;

    keypoints1.clear();
    feature1->findFeatures( frame1, keypoints1 );

    keypoints2.clear();
    feature2->findFeatures( frame2, keypoints2 );
}

void StereoCapture::doCalculations()
{
    CvMat *descriptors1 = feature1->getDescriptor();
    CvMat *descriptors2 = feature2->getDescriptor();

    for ( int i = 0; i < (int)descriptors->size(); i++ )
    {
        cvReleaseMat( &descriptors->at(i) );
    }
    descriptors->clear();
    pos->clear();
    classes->clear();

    for ( int i = 0; i < (int)keypoints1.size(); i++ )
    {
        CvScalar point = keypoints1[i];
        cvCircle( frame1, cvPoint(point.val[0], point.val[1]), 20, cvScalar(255, 0, 0), 2, CV_FILLED);
        cvCircle( frame1, cvPoint(point.val[0], point.val[1]), 4, cvScalar(255, 0, 0), 4, CV_FILLED);
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
        feature1->matchFeatures(descriptors1, classFeatures[i], classMatches);
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
            emit foundNewObject( classLabels[i] );
        }
    }

    for (int i = 0; i < (int)keypoints2.size(); i++)
    {
        CvScalar point = keypoints2[i];
        cvCircle( frame2, cvPoint(point.val[0], point.val[1]), 20, cvScalar(255, 0, 0), 2, CV_FILLED);
        cvCircle( frame2, cvPoint(point.val[0], point.val[1]), 4, cvScalar(255, 0, 0), 4, CV_FILLED);
    }

    vector<CvPoint> matches;
    feature1->matchFeatures(descriptors1, descriptors2, matches);
    feature1->updateThreshold( matches.size() );
    feature2->updateThreshold( matches.size() );

    if ((int)matches.size() > 0)
    {
        CvMat *xl = cvCreateMat( 2, matches.size(), CV_64F );
        CvMat *xr = cvCreateMat( 2, matches.size(), CV_64F );
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

            classes->push_back( classArray[matchix] );

            // Map is transposed !!!
            point = keypoints1[matchix];
            float i0 = (float)point.val[0];
            float i1 = (float)point.val[1];
            float i0f = floor( i0 );
            float i0c = ceil( i0 );
            float i1f = floor( i1 );
            float i1c = ceil( i1 );

            float f1 = (i1-i1f) * (i0-i0f);
            float f2 = (i1-i1f) * (i0c-i0);
            float f3 = (i1c-i1) * (i0-i0f);
            float f4 = (i1c-i1) * (i0c-i0);
            cvmSet( xl, 0, i, ( f1 * cvmGet(mapX_left, i1f, i0f) +
                                f2 * cvmGet(mapX_left, i1f, i0c) +
                                f3 * cvmGet(mapX_left, i1c, i0f) +
                                f4 * cvmGet(mapX_left, i1c, i0c) ) );
            cvmSet( xl, 1, i, ( f1 * cvmGet(mapY_left, i1f, i0f) +
                                f2 * cvmGet(mapY_left, i1f, i0c) +
                                f3 * cvmGet(mapY_left, i1c, i0f) +
                                f4 * cvmGet(mapY_left, i1c, i0c) ) );

            point = keypoints2[matchiy];
            i0 = (float)point.val[0];
            i1 = (float)point.val[1];
            i0f = floor( (float)point.val[0] );
            i0c = ceil( (float)point.val[0] );
            i1f = floor( (float)point.val[1] );
            i1c = ceil( (float)point.val[1] );

            f1 = (i1-i1f) * (i0-i0f);
            f2 = (i1-i1f) * (i0c-i0);
            f3 = (i1c-i1) * (i0-i0f);
            f4 = (i1c-i1) * (i0c-i0);
            cvmSet( xr, 0, i, ( f1 * cvmGet(mapX_right, i1f, i0f) +
                                f2 * cvmGet(mapX_right, i1f, i0c) +
                                f3 * cvmGet(mapX_right, i1c, i0f) +
                                f4 * cvmGet(mapX_right, i1c, i0c) ) );
            cvmSet( xr, 1, i, ( f1 * cvmGet(mapY_right, i1f, i0f) +
                                f2 * cvmGet(mapY_right, i1f, i0c) +
                                f3 * cvmGet(mapY_right, i1c, i0f) +
                                f4 * cvmGet(mapY_right, i1c, i0c) ) );
        }

        stereoTriangulation( xl, xr, pos, this->newT );

        if ( (int)pos->size() == 0 )
        {
            for ( int i = 0; i < (int)descriptors->size(); i++ )
            {
                cvReleaseMat( &descriptors->at( i ) );
            }
            descriptors->clear();
            classes->clear();
        }

        for ( int i = (int)pos->size() - 1; i >= 0; i-- )
        {
//            qDebug( "%f, %f, %f", pos3D[i].val[0], pos3D[i].val[1], pos3D[i].val[2] );
            if ( pos->at( i ).val[2] > 15 || pos->at( i ).val[2] < 1.0 )
            {
                pos->erase( pos->begin() + i );
                cvReleaseMat( &descriptors->at( i ) );
                descriptors->erase( descriptors->begin() + i );
                classes->erase( classes->begin() + i );
            }
        }
    }

    captureMutex.unlock();

    grabFinished();
}

void StereoCapture::normalizePixels(CvMat *x, double f, double cm, double cn)
{
    for ( int i = 0; i < x->cols; i++ )
    {
        cvmSet( x, 0, i, (cvmGet(x, 0, i) - cm) / f );
        cvmSet( x, 1, i, (cvmGet(x, 1, i) - cn) / f );
    }
}

void StereoCapture::stereoTriangulation(CvMat *xl, CvMat *xr, vector<CvScalar> *X, double T)
{
    // Normalize pixels. Normalized pixels will be in homogeneous coordinates.
    normalizePixels( xl, this->f, this->cm, this->cn );
    normalizePixels( xr, this->f, this->cm, this->cn );

    // Do the triangulation.
    double n_ll = 0.0, n_rr = 0.0, n_lT = 0.0, n_rT = 0.0, n_rl = 0.0;
    double DD = 0.0, NN1 = 0.0, NN2 = 0.0;
    double Z1 = 0.0, Z2 = 0.0;
    double X11 = 0.0, X12 = 0.0, X13 = 0.0, X21 = 0.0, X22 = 0.0, X23 = 0.0;
    int nGreaterZero = 0;
    int nSmallerZero = 0;


    for ( int i = 0; i < xl->cols; i++ )
    {
        if ( fabs( cvmGet( xl, 1, i ) - cvmGet( xr, 1, i ) ) > 5 )
        {
            X->push_back( cvScalar( 0, 0, 0 ) );
            nSmallerZero++;
        }
        else
        {
//        qDebug( "xL = [%f, %f]'", cvmGet(xl, 0, i), cvmGet(xl, 1, i) );
//        qDebug( "xR = [%f, %f]'", cvmGet(xr, 0, i), cvmGet(xr, 1, i) );
//        qDebug( "xln = [%f, %f]'", cvmGet(xln, 0, i), cvmGet(xln, 1, i) );
//        qDebug( "xrn = [%f, %f]'", cvmGet(xrn, 0, i), cvmGet(xrn, 1, i) );
            n_ll = cvmGet(xl, 0, i)*cvmGet(xl, 0, i) + cvmGet(xl, 1, i)*cvmGet(xl, 1, i) + 1;
            n_rr = cvmGet(xr, 0, i)*cvmGet(xr, 0, i) + cvmGet(xr, 1, i)*cvmGet(xr, 1, i) + 1;
            n_lT = cvmGet(xl, 0, i) * T; //cvmGet(xln, 0, i)*cvmGet(T, 0, 0) + cvmGet(xln, 1, i)*cvmGet(T, 1, 0) + cvmGet(xln, 2, i)*cvmGet(T, 2, 0);
            n_rT = cvmGet(xr, 0, i) * T; //cvmGet(xrn, 0, i)*cvmGet(T, 0, 0) + cvmGet(xrn, 1, i)*cvmGet(T, 1, 0) + cvmGet(xrn, 2, i)*cvmGet(T, 2, 0);
            n_rl = cvmGet(xl, 0, i)*cvmGet(xr, 0, i) + cvmGet(xl, 1, i)*cvmGet(xr, 1, i) + 1;

            DD = n_ll*n_rr - n_rl*n_rl;
            NN1 = n_rl*n_rT - n_rr*n_lT;
            NN2 = n_ll*n_rT - n_lT*n_rl;

            Z1 = NN1 / DD;
            Z2 = NN2 / DD;

            X11 = cvmGet(xl, 0, i) * Z1;
            X12 = cvmGet(xl, 1, i) * Z1;
            X13 = Z1;
            X21 = cvmGet(xr, 0, i) * (Z2 - T); //cvmGet(xrn, 0, i) * (Z2 - cvmGet(T, 0, 0));
            X22 = cvmGet(xr, 1, i) * Z2; //cvmGet(xrn, 1, i) * (Z2 - cvmGet(T, 1, 0));
            X23 = Z2; //cvmGet(xrn, 2, i) * (Z2 - cvmGet(T, 2, 0));

            if ( X13 + X23 > 0 )
            {
                nGreaterZero++;
            }
            else
            {
                nSmallerZero++;
            }

            X->push_back( cvScalar( 0.5 * (X11 + X21) / 1000,
                                    0.5 * (X12 + X22) / 1000,
                                    0.5 * (X13 + X23) / 1000 ) );
        }
    }

    // Check if left and right were switched.
    if ( (nGreaterZero + nSmallerZero) > 20 && nSmallerZero > nGreaterZero )
    {
        int temp = device1;
        device1 = device2;
        device2 = temp;
        X->clear();
    }
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

vector<CvMat *> *StereoCapture::getDescriptors()
{
    return descriptors;
}

vector<CvScalar> *StereoCapture::getPos()
{
    return pos;
}

vector<int> *StereoCapture::getClasses()
{
    return classes;
}
