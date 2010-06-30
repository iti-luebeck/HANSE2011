#include "feature.h"
#include <time.h>
#include <QDebug>
#include <opencv/cv.h>

#define FEATURE_IMAGE_SCALE     2
#define FEATURE_P_THRESHOLD     0.8

Feature::Feature()
{
    storage = cvCreateMemStorage(0);
    params = cvSURFParams(2000, 0);
    surfThreshold = 0.002;
    descriptor = NULL;
    fastThreshold = 100;
    keypoints = NULL;
}

Feature::~Feature()
{
    cvReleaseMemStorage(&storage);
}

void Feature::run()
{
    if ( keypoints != NULL )
    {
        IplImage *copy = cvCreateImage( cvSize( image->width / FEATURE_IMAGE_SCALE, image->height / FEATURE_IMAGE_SCALE ),
                                        image->depth, image->nChannels );
        cvResize( image, copy, CV_INTER_LINEAR );
        IpVec ipts;
        surfDetDes( copy, ipts, false, 4, 4, 2, surfThreshold );

    //    // Convert images to grayscale.
    //    IplImage *copy_gray = cvCreateImage( cvGetSize( copy ), IPL_DEPTH_8U, 1 );
    //    cvCvtColor( copy, copy_gray, CV_RGB2GRAY );
    //
    //    // Extract interest points.
    //    CvPoint* corners;
    //    int numCorners;
    //    cvCornerFast( copy_gray, (int)fastThreshold, 9, 1, &numCorners, &corners );
    //
    //    // Calculate SURF descriptors for the left image.
    //    IpVec ipts;
    //    for ( int i = 0; i < numCorners; i++ )
    //    {
    //        CvPoint p = corners[i];
    //        Ipoint ip;
    //        ip.x = p.x;
    //        ip.y = p.y;
    //        ip.scale = 3.5;
    //        ipts.push_back( ip );
    //    }
    //    surfDes( image, ipts, true );

        numFeatures = ipts.size();
        if ( numFeatures >= 1 )
        {
            if ( descriptor ) cvReleaseMat( &descriptor );
            descriptor = cvCreateMat( numFeatures, 64, CV_32F );
            for ( int i = 0; i < numFeatures; i++ )
            {
                Ipoint point = ipts[i];
                for ( int j = 0; j < 64; j++ )
                {
                    cvmSet( descriptor, i, j, point.descriptor[j] );
                }
                keypoints->push_back( cvScalar( FEATURE_IMAGE_SCALE * point.x, FEATURE_IMAGE_SCALE * point.y ) );
            }
            cvReleaseImage( &copy );
        }
        else
        {
            cvReleaseImage( &copy );
        }
    }
}

void Feature::findFeatures(IplImage *image, CvSeq **keypoints, CvSeq **descriptors)
{
    if (image->nChannels == 3)
    {
        IplImage *imageGray = cvCreateImage( cvSize(image->width, image->height), IPL_DEPTH_8U, 1 );
        cvCvtColor( image, imageGray, CV_RGB2GRAY );
        cvExtractSURF( imageGray, NULL, keypoints, descriptors, storage, params );
        cvReleaseImage( &imageGray );
    }
    else
    {
        cvExtractSURF( image, NULL, keypoints, descriptors, storage, params );
    }
}

void Feature::findFeatures(IplImage *image, vector<CvScalar> &keypoints)
{
    this->image = image;
    this->keypoints = &keypoints;
    this->start( QThread::LowPriority );
}

void Feature::updateThreshold(int numFeatures)
{
    if (numFeatures <= 10)
    {
//        params.hessianThreshold = FEATURE_P_THRESHOLD * params.hessianThreshold;
        surfThreshold = FEATURE_P_THRESHOLD * surfThreshold;
//        fastThreshold = fastThreshold -= 5;
    }
    else if (numFeatures >= 20)
    {
//        params.hessianThreshold = params.hessianThreshold / FEATURE_P_THRESHOLD;
        surfThreshold = surfThreshold / FEATURE_P_THRESHOLD;
//        fastThreshold = fastThreshold += 5;
    }
}

void Feature::matchFeatures(CvSeq *descriptors1, CvSeq *descriptors2, vector<CvPoint> &matches)
{
    if (descriptors1 != NULL && descriptors2 != NULL && descriptors1->total > 0 && descriptors2->total > 0)
    {
        int length = (int)(descriptors1->elem_size/sizeof(float));

        cv::Mat m_object( descriptors1->total, length, CV_32F );
        cv::Mat m_image( descriptors2->total, length, CV_32F );

        float *obj_ptr = m_object.ptr<float>( 0 );
        for ( int i = 0; i < descriptors1->total; i++ )
        {
            const float* descriptor = (const float *)cvGetSeqElem(descriptors1, i);
            memcpy( obj_ptr, descriptor, length*sizeof(float) );
            obj_ptr += length;
        }

        float *img_ptr = m_image.ptr<float>( 0 );
        for ( int i = 0; i < descriptors2->total; i++ )
        {
            const float* descriptor = (const float *)cvGetSeqElem(descriptors2, i);
            memcpy( img_ptr, descriptor, length*sizeof(float) );
            img_ptr += length;
        }

        // Find the two nearest neighbors using FLANN.
        cv::Mat m_indices(descriptors1->total, 2, CV_32S);
        cv::Mat m_dists(descriptors1->total, 2, CV_32F);
        cv::flann::Index flann_index( m_image, cv::flann::KDTreeIndexParams(4) );  // using 4 randomized kdtrees
        flann_index.knnSearch( m_object, m_indices, m_dists, 2, cv::flann::SearchParams(64) ); // maximum number of leafs checked

        // We found a match if the score of the nearest neighbor is less than
        // 0.6 times that of the second nearest neighbor
        int *indices_ptr = m_indices.ptr<int>( 0 );
        float *dists_ptr = m_dists.ptr<float>( 0 );
        for ( int i = 0; i < m_indices.rows; ++i ) {
//            qDebug( "d%d = ( %f, %f )", i, dists_ptr[2*i], dists_ptr[2*i+1] );
            if ( dists_ptr[2*i] < 0.5*dists_ptr[2*i+1] ) {
                CvPoint p = {i, indices_ptr[2*i]};
                matches.push_back(p);
            }
        }
    }
}

void Feature::matchFeatures( CvMat *descriptors1, CvMat *descriptors2, vector<CvPoint> &matches )
{
    if ( descriptors1 != NULL && descriptors2 != NULL && descriptors1->height > 0 && descriptors2->height > 0 )
    {        
//        qDebug( "d1 %dx%d, d2 %dx%d", descriptors1->height, descriptors1->width,
//                descriptors2->height, descriptors2->width );
//        cv::Mat features( descriptors1 );
//        features = features.t();
//        cv::Mat objectFeatures( descriptors2 );
//        objectFeatures = objectFeatures.t();
//
//        for (int i = 0; i < features.cols; i++)
//        {
//            int bestk = -1;
//            double best = 1e20;
//            double secondBest = 1e20;
//            cv::Mat feature = cv::Mat(64, 1, CV_32F, cv::Scalar(0));
//            feature = feature + features.col(i);
//            for (int j = 0; j < objectFeatures.cols; j++)
//            {
//                cv::Mat objectFeature = cv::Mat(64, 1, CV_32F, cv::Scalar(0));
//                objectFeature = objectFeature + objectFeatures.col(j);
//                double acc = norm(feature - objectFeature, cv::NORM_L2);
//
//                if (acc < best)
//                {
//                    secondBest = best;
//                    best = acc;
//                    bestk = j;
//                }
//                else if (acc < secondBest)
//                {
//                    secondBest = acc;
//                }
//            }
//
//            //qDebug( "d%d = ( %f, %f )", i, best, secondBest );
//            if (best < 0.6 * secondBest && bestk >= 0)
//            {
//                CvPoint p = {i, bestk};
//                matches.push_back(p);
//            }
//        }

        cv::Mat m_object( descriptors1 );
        cv::Mat m_image( descriptors2 );

        // Find the two nearest neighbors using FLANN.
        cv::Mat m_indices(descriptors1->height, 2, CV_32S);
        cv::Mat m_dists(descriptors1->height, 2, CV_32F);
        cv::flann::Index flann_index(m_image, cv::flann::KDTreeIndexParams(4));  // using 4 randomized kdtrees
        //cv::flann::Index bf_index( m_image, cv::flann::LinearIndexParams() );
        flann_index.knnSearch( m_object, m_indices, m_dists, 2, cv::flann::SearchParams( 64 ) ); // maximum number of leafs checked

        // We found a match if the score of the nearest neighbor is less than
        // 0.6 times that of the second nearest neighbor
        int *indices_ptr = m_indices.ptr<int>( 0 );
        float *dists_ptr = m_dists.ptr<float>( 0 );
        for ( int i = 0; i < m_indices.rows; ++i ) {
            //qDebug( "d%d = ( %f, %f )", i, dists_ptr[2*i], dists_ptr[2*i+1] );
            if ( dists_ptr[2*i] < 0.6*dists_ptr[2*i+1] ) {
                CvPoint p = {i, indices_ptr[2*i]};
                matches.push_back(p);
            }
        }
    }
}

/*
CvMat *Feature::matchFeatures( IplImage *image1, IplImage *image2, vector<CvScalar> &keypoints1,
                                    vector<CvScalar> &keypoints2 )
{
    clock_t start, stop;

    start = clock();
    // Convert images to grayscale.
    IplImage *img1_gray = cvCreateImage( cvGetSize( image1 ), IPL_DEPTH_8U, 1 );
    cvCvtColor( image1, img1_gray, CV_RGB2GRAY );
    IplImage *img2_gray = cvCreateImage( cvGetSize( image2 ), IPL_DEPTH_8U, 1 );
    cvCvtColor( image2, img2_gray, CV_RGB2GRAY );

    // Extract interest points.
    CvPoint* corners1;
    int numCorners1;
    cvCornerFast( img1_gray, 100, 9, 0, &numCorners1, &corners1 );
    CvPoint* corners2;
    int numCorners2;
    cvCornerFast( img2_gray, 100, 9, 0, &numCorners2, &corners2 );
    stop = clock();
    qDebug( "  INTEREST POINTS %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );


    //-----------------------------------------------------------------------------------------------------
    // Calculate matches using the Calonder descriptor.
    start = clock();
    int patch_width = cv::PATCH_SIZE;
    int patch_height = cv::PATCH_SIZE;
    vector<cv::BaseKeypoint> base_set;
    for ( int i = 0; i < numCorners1; i++ )
    {
        base_set.push_back( cv::BaseKeypoint( corners1[i].x, corners1[i].y, img1_gray ) );
    }

    //Detector training
    cv::RNG rng( cvGetTickCount() );
    cv::PatchGenerator gen(0, 255, 2, false, 0.7, 1.3, -CV_PI/3, CV_PI/3, -CV_PI/3, CV_PI/3 );

    qDebug( "RTree Classifier training..." );
    detector.train( base_set, rng, gen, 24, cv::DEFAULT_DEPTH, 100,
                   (int)base_set.size(), detector.DEFAULT_NUM_QUANT_BITS, true );
    qDebug( "Done" );

    float *signature = new float[detector.original_num_classes()];
    float *best_corr;
    int* best_corr_idx;
    if ( numCorners2 > 0 )
    {
        best_corr = new float[numCorners2];
        best_corr_idx = new int[numCorners2];
    }

    for( int i = 0; i < numCorners2; i++ )
    {
        int part_idx = -1;
        float prob = 0.0f;

        CvRect roi = cvRect( (int)(corners2[i].x) - patch_width/2,
                             (int)(corners2[i].y) - patch_height/2,
                             patch_width, patch_height );
        cvSetImageROI( img2_gray, roi );
        roi = cvGetImageROI( img2_gray );
        if ( roi.width != patch_width || roi.height != patch_height )
        {
            best_corr_idx[i] = part_idx;
            best_corr[i] = prob;
        }
        else
        {
            cvSetImageROI( img2_gray, roi );
            IplImage *roi_image = cvCreateImage( cvSize(roi.width, roi.height),
                                                 img2_gray->depth, img2_gray->nChannels );
            cvCopy( img2_gray, roi_image );

            detector.getSignature( roi_image, signature );
            for ( int j = 0; j < detector.original_num_classes(); j++ )
            {
                if (prob < signature[j])
                {
                    part_idx = j;
                    prob = signature[j];
                }
            }

            best_corr_idx[i] = part_idx;
            best_corr[i] = prob;


            if (roi_image)
                cvReleaseImage(&roi_image);
        }
        cvResetImageROI(img2_gray);
    }    
    //-----------------------------------------------------------------------------------------------------
    stop = clock();
    qDebug( "  CALONDER FEATURES %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

    cvReleaseImage( &img1_gray );
    cvReleaseImage( &img2_gray );

    // Calculate SURF descriptors for the left image.
    start = clock();
    IpVec ipts;
    for ( int i = 0; i < numCorners1; i++ )
    {
        CvPoint p = corners1[i];
        Ipoint ip;
        ip.x = p.x;
        ip.y = p.y;
        ip.scale = 1;
        ipts.push_back( ip );
    }
    surfDes( image1, ipts, false );
    stop = clock();
    qDebug( "  SURF DESCRIPTORS %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

    // Copy descriptors to a matrix and return that matrix. Add keypoints to the keypoint vectors.
    int numFeatures = ipts.size();
    if ( numFeatures >= 1 )
    {
        CvMat *des = cvCreateMat( numFeatures, 64, CV_32F );
        for ( int i = 0; i < numFeatures; i++ )
        {
            Ipoint point = ipts[i];
            for ( int j = 0; j < 64; j++ )
            {
                cvmSet( des, i, j, point.descriptor[j] );
            }
            keypoints1.push_back( cvScalar( point.x, point.y ) );
        }
        return des;
    }
    else
    {
        return NULL;
    }
}
*/

CvMat *Feature::getDescriptor()
{
    return descriptor;
}
