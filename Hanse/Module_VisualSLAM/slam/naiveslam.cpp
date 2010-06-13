#include "naiveslam.h"
#include <Module_VisualSLAM/feature/feature.h>
#include <Module_VisualSLAM/helpers.h>
#include <iostream>
#include <QGraphicsTextItem>

NaiveSLAM::NaiveSLAM()
{
    storage = cvCreateMemStorage( 0 );
    features = cvCreateSeq( 0, sizeof(CvSeq), 64*sizeof(float), storage );
    inited = false;

    currentTranslation = cvCreateMat( 3, 1, CV_32F );
    cvSetZero( currentTranslation );
    lastTranslation = cvCreateMat( 3, 1, CV_32F );
    cvSetZero( lastTranslation );

    // Initialize FastSLAM parameters.
    Robservation = cvCreateMat( 3, 3, CV_32F );
    cvSetIdentity( Robservation, cvScalar( 0.09 ) );

    Rstate = cvCreateMat( 7, 7, CV_32F );
    cvSetIdentity( Rstate, cvScalar( 0.09 ) );
    cvmSet( Rstate, 0, 0, 0.25 );
    cvmSet( Rstate, 1, 1, 0.25 );
    cvmSet( Rstate, 2, 2, 0.25 );

    Gobservation = cvCreateMat( 3, 3, CV_32F );
    Gstate = cvCreateMat( 3, 7, CV_32F );

    P3 = cvCreateMat( 3, 3, CV_32F );
    Q3 = cvCreateMat( 3, 3, CV_32F );
    K = cvCreateMat( 3, 3, CV_32F );
    R = cvCreateMat( 3, 3, CV_32F );
    U = cvCreateMat( 3, 3, CV_32F );
    D = cvCreateMat( 3, 3, CV_32F );
    V = cvCreateMat( 3, 3, CV_32F );
    meanLocalPosition = cvCreateMat( 3, 1, CV_32F );
    meanGlobalPosition = cvCreateMat( 3, 1, CV_32F );
}

NaiveSLAM::~NaiveSLAM()
{
    cvReleaseMemStorage( &storage );
    cvReleaseMat( &currentTranslation );
    cvReleaseMat( &lastTranslation );
    cvReleaseMat( &Robservation );
    cvReleaseMat( &Rstate );
    cvReleaseMat( &Gobservation );
    cvReleaseMat( &Gstate );
    cvReleaseMat( &P3 );
    cvReleaseMat( &Q3 );
    cvReleaseMat( &K );
    cvReleaseMat( &R );
    cvReleaseMat( &U );
    cvReleaseMat( &D );
    cvReleaseMat( &V );
    cvReleaseMat( &meanLocalPosition );
    cvReleaseMat( &meanGlobalPosition );
}

bool NaiveSLAM::update( vector<CvMat *> descriptor, vector<CvScalar> &pos3D, vector<CvScalar> &pos2D, vector<int> classLabels )
{
//    clock_t start, stop;
//    start = clock();
    int totalFeatures = (int)descriptor.size();
    bool success = false;

    if ( totalFeatures > 0 )
    {
        // Copy descriptor to sequence.
        CvSeq *newFeatures = cvCreateSeq( 0, sizeof(CvSeq), 64*sizeof(float), storage );
        for ( int i = 0; i < totalFeatures; i++ )
        {
            cvSeqPush(newFeatures, descriptor[i]->data.fl);
        }

        // Check which of the new features are in the map.
        vector<CvPoint> mapMatches;
        Feature::matchFeatures( newFeatures, features, mapMatches );

        // Check which of the new features were found in the map.
        bool *found = new bool[totalFeatures];
        memset( found, false, sizeof(bool) );
        for ( int i = 0; i < (int)mapMatches.size(); i++ )
        {
            found[mapMatches[i].x] = true;
        }
//        stop = clock();
//        qDebug( "UPDATE INIT %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

        // Determine current Position from the matched features. This will
        // only work if there are at least 3 matches.
        if ( mapMatches.size() >= 3 )
        {
            success = updatePosition( pos3D, mapMatches );
        }

        // Update the map with the new Features.
        updateMap( descriptor, pos3D, found, mapMatches, classLabels );

        delete( found );

        float yaw;
        float pitch;
        float roll;
        currentRotation.getYawPitchRoll( yaw, pitch, roll );
        double x = cvmGet( currentTranslation, 0, 0 );
        double y = cvmGet( currentTranslation, 1, 0 );
        double z = cvmGet( currentTranslation, 2, 0 );
        pos = Position( x, y, z, roll, pitch, yaw );
    }

    return success;
}

bool NaiveSLAM::updatePosition( vector<CvScalar> newPositions, vector<CvPoint> matches )
{
    bool success = true;
//    clock_t start, stop;
//    start = clock();

    lastRotation = currentRotation;    
    qDebug( "Last Rotation:" );
    qDebug( "%s", lastRotation.toString().toStdString().c_str() );
    cvCopy( currentTranslation, lastTranslation );

    int numPositions = (int)newPositions.size();
    bool *maxConsensusSet = new bool[numPositions];
    bool *tempConsensusSet = new bool[numPositions];
    int maxConsensus = 0;
    int tempMaxConsensus = 0;
    for ( int i = 0; i < 100; i++ )
    {
        // Calculate position with three random samples.
        memset( tempConsensusSet, false, numPositions*sizeof(bool) );
        drawRandomSamples( numPositions, tempConsensusSet );
        calcPosition( newPositions, matches, tempConsensusSet, 3 );

        // Check which samples are inside the threshold.
        errorGreaterT( newPositions, matches, 0.5, tempConsensusSet, tempMaxConsensus );

        // Update maximum consensus set if the consensus is higher.
        if ( tempMaxConsensus > maxConsensus )
        {
            maxConsensus = tempMaxConsensus;
            memcpy( maxConsensusSet, tempConsensusSet, numPositions*sizeof(bool) );

            if ( maxConsensus > (int)(0.8 * ((double)numPositions)) )
            {
                break;
            }
        }
    }

    // Calculate rotation and translation for maximum consensus set.
    qDebug( "maximum consensus: %d / %d", maxConsensus, numPositions );
    if ( maxConsensus >= 3 )
    {
        calcPosition( newPositions, matches, maxConsensusSet, maxConsensus );
        qDebug( "Rotation:" );
        qDebug( "%s", currentRotation.toString().toStdString().c_str() );
        qDebug( "Translation:" );
        qDebug( "%1.4f %1.4f %1.4f", cvmGet( currentTranslation, 0, 0 ), cvmGet( currentTranslation, 1, 0 ), cvmGet( currentTranslation, 2, 0 ) );

        if ( inited )
        {
            // Check if the calculated translation and rotation seem sensible.
            double normTranslation = cvNorm( lastTranslation, currentTranslation );
            Quaternion diffRotation = lastRotation - currentRotation;
            double normRotation = diffRotation.norm();
//            qDebug( "Td = %f, Rd = %f", normTranslation, normRotation );
            if ( normTranslation > 2.0 || normRotation > 0.7 )
            {
                success = false;
                qDebug( "No SUCCESS: R = %f, t = %f", normRotation, normTranslation );
            }
        }
        else
        {
            inited = true;
        }
    }
    else
    {
        success = false;
    }

    // !!!!!
    //success = false;

    // Ignore current step if the position could not be obtained.
    if ( !success )
    {
        currentRotation = lastRotation;
        cvCopy( lastTranslation, currentTranslation );
    }

    qDebug( "Rotation:" );
    qDebug( "%s", currentRotation.toString().toStdString().c_str() );
    qDebug( "Translation:" );
    qDebug( "%1.4f %1.4f %1.4f", cvmGet( currentTranslation, 0, 0 ), cvmGet( currentTranslation, 1, 0 ), cvmGet( currentTranslation, 2, 0 ) );

    delete( tempConsensusSet );
    delete( maxConsensusSet );

//    stop = clock();
//    qDebug( "NAIVE UPDATE %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

    return success;
}

void NaiveSLAM::drawRandomSamples( int num, bool *selected )
{
    int count = 0;
    while (count != 3)
    {
        int r = rand() % num;
        if ( !selected[r] )
        {
//            qDebug( "%d", r );
            selected[r] = true;
            count++;
        }
    }
}

void NaiveSLAM::calcPosition( vector<CvScalar> newPositions, vector<CvPoint> matches, bool *selected, int num )
{
    cvZero( meanLocalPosition );
    cvZero( meanGlobalPosition );
    CvMat *P;
    CvMat *Q;

    if ( num == 3 )
    {
        P = P3;
        Q = Q3;
    }
    else
    {
        P = cvCreateMat( 3, num, CV_32F );
        Q = cvCreateMat( 3, num, CV_32F );
    }

    int k = 0;
    for ( int i = 0; i < (int) matches.size(); i++ )
    {
//        qDebug("%d", i);
//        qDebug( "%1.4f %1.4f %1.4f", localPositions[i].val[0], localPositions[i].val[1], localPositions[i].val[2] );
//        qDebug( "%1.4f %1.4f %1.4f", globalPositions[i].val[0], globalPositions[i].val[1], globalPositions[i].val[2] );
        if ( selected[i] )
        {
            for ( int j = 0; j < 3; j++ )
            {
                cvmSet( P, j, k, newPositions[matches[i].x].val[j] );
                cvmSet( Q, j, k, landmarks[matches[i].y]->getPos(j) );

                cvmSet( meanLocalPosition, j, 0, cvmGet( meanLocalPosition, j, 0 ) + cvmGet( P, j, k ) );
                cvmSet( meanGlobalPosition, j, 0, cvmGet( meanGlobalPosition, j, 0 ) + cvmGet( Q, j, k ) );
            }
            k++;
        }
    }

    for ( int j = 0; j < 3; j++ )
    {
        cvmSet( meanLocalPosition, j, 0, cvmGet( meanLocalPosition, j, 0 ) / num );
        cvmSet( meanGlobalPosition, j, 0, cvmGet( meanGlobalPosition, j, 0 ) / num );
    }

    for ( k = 0; k < num; k++ )
    {
        for ( int j = 0; j < 3; j++ )
        {
            cvmSet( P, j, k, cvmGet( P, j, k ) - cvmGet( meanLocalPosition, j, 0 ) );
            cvmSet( Q, j, k, cvmGet( Q, j, k ) - cvmGet( meanGlobalPosition, j, 0 ) );
        }
    }
//    qDebug( "Q" );
//    printMatrix( Q );
//    qDebug( "P" );
//    printMatrix( P );

    cvGEMM( Q, P, 1, NULL, 1, K, CV_GEMM_B_T );

    cvSVD( K, D, U, V, CV_SVD_MODIFY_A | CV_SVD_V_T );

    cvSetIdentity( D, cvScalar(1) );
    cvMatMul( U, D, R );
    cvMatMul( R, V, R );

//    qDebug( "R" );
//    printMatrix( R );

    cvMatMul( R, meanLocalPosition, currentTranslation );
    cvSub( meanGlobalPosition, currentTranslation, currentTranslation );

    currentRotation = Quaternion::fromRotation( R );

    if ( num != 3 )
    {
        cvReleaseMat( &P );
        cvReleaseMat( &Q );
    }
}

void NaiveSLAM::errorGreaterT( vector<CvScalar> newPositions, vector<CvPoint> matches, double T, bool *check, int &count )
{
    count = 0;
    CvMat *tempLocalPosition = cvCreateMat( 3, 1, CV_32F );
    CvMat *tempGlobalPosition;
    for ( int i = 0; i < (int)matches.size(); i++ )
    {
        for ( int j = 0; j < 3; j++ )
        {
            cvmSet( tempLocalPosition, j, 0, newPositions[matches[i].x].val[j] );
        }
        tempGlobalPosition = landmarks[matches[i].y]->getPos();
        //printMatrix( tempGlobalPosition, "global position" );
        Quaternion::rotate( currentRotation, tempLocalPosition );
        cvAdd( tempLocalPosition, currentTranslation, tempLocalPosition );
        //printMatrix( tempLocalPosition, "local position" );
        //cvMatMulAdd( this->currentRotation, tempLocalPosition, this->currentTranslation, temp );
        double norm = cvNorm( tempLocalPosition, tempGlobalPosition );

        if ( norm <= T )
        {
            check[i] = true;
            count++;
        }
        else
        {
            check[i] = false;
        }
    }
    cvReleaseMat( &tempLocalPosition );
}

void NaiveSLAM::updateMap( vector<CvMat *> newFeatures, vector<CvScalar> newPositions,
                           bool *found, vector<CvPoint> matches, vector<int> classLabels )
{
    CvMat *newFeature;
    CvMat *newPosition = cvCreateMat( 3, 1, CV_32F );
    CvMat *expectedPosition = cvCreateMat( 3, 1, CV_32F );
    CvMat *R = currentRotation.getRotation();
    int oldLandmarkCount = (int)landmarks.size();
    bool *toDelete = new bool[oldLandmarkCount];
    memset( toDelete, false, oldLandmarkCount * sizeof(bool) );
//    clock_t start, stop;
//    start = clock();

    // obj.Sigma = Rt;
    CvMat *Sigma = cvCreateMat( 7, 7, CV_32F );
    cvCopy( Rstate, Sigma );

    // mus = obj.s;
    CvMat *sfilt = cvCreateMat( 7, 1, CV_32F );
    cvmSet( sfilt, 0, 0, cvmGet( currentTranslation, 0, 0 ) );
    cvmSet( sfilt, 1, 0, cvmGet( currentTranslation, 1, 0 ) );
    cvmSet( sfilt, 2, 0, cvmGet( currentTranslation, 2, 0 ) );
    cvmSet( sfilt, 3, 0, currentRotation.w );
    cvmSet( sfilt, 4, 0, currentRotation.x );
    cvmSet( sfilt, 5, 0, currentRotation.y );
    cvmSet( sfilt, 6, 0, currentRotation.z );

    CvMat *Z = cvCreateMat( 3, 3, CV_32F );
    CvMat *Zinv = cvCreateMat( 3, 3, CV_32F );
    CvMat *Sigmainv = cvCreateMat( 7, 7, CV_32F );
    CvMat *Mtemp1 = cvCreateMat( 7, 3, CV_32F );
    CvMat *diffPos = cvCreateMat( 3, 1, CV_32F );

    // Build proposal distribution.
    // for j = 1:length(matches)
    int count = 0;
    confidence = 1.0;
    for ( int i = 0; i < (int)matches.size(); i++ )
    {
        cvmSet( newPosition, 0, 0, newPositions[matches[i].x].val[0] );
        cvmSet( newPosition, 1, 0, newPositions[matches[i].x].val[1] );
        cvmSet( newPosition, 2, 0, newPositions[matches[i].x].val[2] );

        // Expected measurement.
        // zhat = quat(obj.s(4:7))' * (obj.mu(:,matches(i)) - obj.s(1:3));
        cvSub( landmarks[matches[i].y]->getPos(), currentTranslation, expectedPosition );
        cvGEMM( R, expectedPosition, 1, NULL, 1, expectedPosition, CV_GEMM_A_T );

        // [ Gs, Gtheta ] = obj.jacobians(obj.mu(:,matches(i)), z(:,i), zhat);
        getObservationJacobian( Gobservation );
        getStateJacobian( matches[i].y, Gstate );

//        if z(3,i) > 2.0
//            Roprime = ( z(3,i) / 2.0 ) * Ro;
//        else
//            Roprime = Ro;
//        end
        // Z = Roprime + Gtheta * obj.sigma(:,(3*matches(i)-2):(3*matches(i))) * Gtheta';
        cvGEMM( Gobservation, landmarks[matches[i].y]->getSigma(), 1, NULL, 1, Z );
        cvGEMM( Z, Gobservation, 1, Robservation, 1, Z, CV_GEMM_B_T );

        // pi = mvnpdf(z(:,i),quat(obj.s(4:7))' * ((obj.mu(:,matches(i)) - obj.s(1:3))), Z);
        cvSub( newPosition, expectedPosition, diffPos );
        double p = mvnpdf( diffPos, NULL, Z );

//        if ps(i) > obj.p0
//            obj.Sigma = inv(Gs' * inv(Z) * Gs + inv(obj.Sigma));
//            mus = mus + obj.Sigma * Gs' * inv(Z) * (z(:,i) - zhat);
//            mus(4:7) = mus(4:7) / norm(mus(4:7));
//        else
//            obj.valid(matches(i)) = false;
//        end
        double p0 = 0.01;
        if ( p > p0 )
        {
            cvInv( Z, Zinv, CV_SVD );
            cvInv( Sigma, Sigmainv, CV_SVD );
            cvGEMM( Gstate, Zinv, 1, NULL, 1, Mtemp1, CV_GEMM_A_T );
            cvGEMM( Mtemp1, Gstate, 1, Sigmainv, 1, Sigmainv );
            cvInv( Sigmainv, Sigma );

            cvGEMM( Sigma, Gstate, 1, NULL, 1, Mtemp1, CV_GEMM_B_T );
            cvGEMM( Mtemp1, Zinv, 1, NULL, 1, Mtemp1 );
            cvGEMM( Mtemp1, diffPos, 1, sfilt, 1, sfilt );
            count++;


            cvmSet( currentTranslation, 0, 0, cvmGet( sfilt, 0, 0 ) );
            cvmSet( currentTranslation, 1, 0, cvmGet( sfilt, 1, 0 ) );
            cvmSet( currentTranslation, 2, 0, cvmGet( sfilt, 2, 0 ) );
            currentRotation.w = cvmGet( sfilt, 3, 0 );
            currentRotation.x = cvmGet( sfilt, 4, 0 );
            currentRotation.y = cvmGet( sfilt, 5, 0 );
            currentRotation.z = cvmGet( sfilt, 6, 0 );
            currentRotation.normalize();

            confidence *= p;
        }
        else
        {
            toDelete[matches[i].y] = true;
            confidence *= p0;
        }
    }

    cvReleaseMat( &Sigma );
    cvReleaseMat( &Sigmainv );
    cvReleaseMat( &sfilt );
    cvReleaseMat( &Z );
    cvReleaseMat( &Zinv );
    cvReleaseMat( &Mtemp1 );
    cvReleaseMat( &diffPos );

//    stop = clock();
//    qDebug( "FAST FILTER %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

//    start = clock();
    for ( int i = 0; i < (int)matches.size(); i++ )
    {
        cvmSet( newPosition, 0, 0, newPositions[matches[i].x].val[0] );
        cvmSet( newPosition, 1, 0, newPositions[matches[i].x].val[1] );
        cvmSet( newPosition, 2, 0, newPositions[matches[i].x].val[2] );

        cvSub( landmarks[matches[i].y]->getPos(), currentTranslation, expectedPosition );
        cvGEMM( R, expectedPosition, 1, NULL, 1, expectedPosition, CV_GEMM_A_T );

        getObservationJacobian( Gobservation );
        getStateJacobian( matches[i].y, Gstate );
        landmarks[matches[i].y]->update( newPosition, expectedPosition,
                                         Robservation, Rstate,
                                         Gobservation, Gstate );
    }
//    stop = clock();
//    qDebug( "EKF UPDATE %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

//    start = clock();
    for ( int i = 0; i < (int)newFeatures.size(); i++ )
    {
        if ( !found[i] )
        {
            newFeature = newFeatures[i];
            // Calculate global position of the feature.
            cvmSet( newPosition, 0, 0, newPositions[i].val[0] );
            cvmSet( newPosition, 1, 0, newPositions[i].val[1] );
            cvmSet( newPosition, 2, 0, newPositions[i].val[2] );
            cvMatMulAdd( R, newPosition, currentTranslation, newPosition );

            cvSeqPush( features, newFeature->data.fl );

            landmarks.push_back( new Landmark( newPosition, Robservation, (int)landmarks.size(), classLabels[i]) );
        }
    }
//    stop = clock();
//    qDebug( "NEW LANDMARKS %0.3f msec", (double)(1000 * (stop - start) / CLOCKS_PER_SEC) );

    // Delete bad landmarks.
    /*
    for ( int i = oldLandmarkCount - 1; i >= 0; i-- )
    {
        if ( toDelete[i] )
        {
            delete( landmarks[i] );
            landmarks.erase( landmarks.begin() + i );
            cvSeqRemove( features, i );
        }
    }
    */

    delete( toDelete );
    cvReleaseMat( &newPosition );
    cvReleaseMat( &expectedPosition );
    cvReleaseMat( &R );
}

void NaiveSLAM::getObservationJacobian( CvMat *Gobservation )
{
    CvMat *R = currentRotation.getRotation();
    cvCopy( R, Gobservation );
    cvTranspose( Gobservation, Gobservation );
    cvReleaseMat( &R );
}

void NaiveSLAM::getStateJacobian( int landmarkNr, CvMat *Gstate )
{
    CvMat *diffPos = cvCreateMat( 3, 1, CV_32F );
    cvSub( landmarks[landmarkNr]->getPos(), currentTranslation, diffPos );
    double d1 = cvmGet( diffPos, 0, 0 );
    double d2 = cvmGet( diffPos, 1, 0 );
    double d3 = cvmGet( diffPos, 2, 0 );
    double a = currentRotation.w;
    double b = currentRotation.x;
    double c = currentRotation.y;
    double d = currentRotation.z;

    cvSetZero( Gstate );

    CvMat *R = currentRotation.getRotation();
    cvTranspose( R, Gobservation );
    for ( int i = 0; i < 3; i++ )
    {
        for ( int j = 0; j < 3; j++ )
        {
            cvmSet( Gstate, i, j, -cvmGet( Gobservation, i, j ) );
        }
    }
    cvReleaseMat( &R );

    float Gs_A[3][4] = { { ( d2*d - d3*c), (         d2*c +   d3*d), (-2*d1*c + d2*b -   d3*a), (-2*d1*d +   d2*a + d3*b) },
                         { (-d1*d + d3*b), (d1*c - 2*d2*b +   d3*a), (   d1*b +          d3*d), (  -d1*a - 2*d2*d + d3*c) },
                         { ( d1*c - d2*b), (d1*d -   d2*a - 2*d3*b), (   d1*a + d2*d - 2*d3*c), (   d1*b +   d2*c       ) } };
    for ( int i = 0; i < 3; i++ )
    {
        for ( int j = 0; j < 4; j++ )
        {
            cvmSet( Gstate, i, j+3, 2 * Gs_A[i][j] );
        }
    }
}

Position NaiveSLAM::getPosition()
{
    return pos;
}

double NaiveSLAM::getConfidence()
{
    return confidence;
}

void NaiveSLAM::plot( QGraphicsScene *scene )
{
    double w = scene->width();
    double h = scene->height();

    QPen pen(Qt::red);
    QBrush brush(Qt::red);
    pen.setWidth( 1.0 );
    double pos1 = cvmGet( this->currentTranslation, 0, 0 );
    double pos2 = -cvmGet( this->currentTranslation, 2, 0 );
    CvMat *R = currentRotation.getRotation();
    double z1 = cvmGet( R, 2, 0 );
    double z2 = cvmGet( R, 2, 2 );
    cvReleaseMat( &R );
//    scene->addEllipse( (w / (2*max))*(p1 + max) - 5,
//                       (h / (max))*(max - p2) - 5,
//                       10, 10, pen, brush );
//    scene->addLine( (w / (2*max))*(p1 + max),
//                    (h / (max))*(max - p2),
//                    (w / (2*max))*(p1 - z1 + max),
//                    (h / (max))*(max - (p2 + z2)),
//                    pen );
    scene->addEllipse( w/2 + 10*pos1 - 0.5,
                       h/2 + 10*pos2 - 0.5,
                       1, 1, pen, brush );
    scene->addLine( w/2 + 10*pos1,
                    h/2 + 10*pos2,
                    w/2 + 10*(pos1 - z1),
                    h/2 + 10*(pos2 - z2),
                    pen );

    QFont font( "Arial", 1 );
    QGraphicsTextItem *textItem = scene->addText( QString( "(%1,%2)" ).arg( pos1, 0, 'f', 2 ).arg( pos2, 0, 'f', 2 ), font );
    textItem->setPos( w/2 + 10*pos1 - textItem->boundingRect().width()/2, h/2 + 10*pos2 + 0.5 );

    for ( int i = 0; i < (int)landmarks.size(); i++ )
    {
        switch ( landmarks[i]->getClass() )
        {
        case 0:
            pen = QPen(Qt::blue);
            brush = QBrush(Qt::blue);
            break;
        case 1:
            pen = QPen(Qt::green);
            brush = QBrush(Qt::green);
            break;
        case 2:
            pen = QPen(Qt::red);
            brush = QBrush(Qt::red);
            break;
        }

//        scene->addEllipse( (w / (2*max))*(landmarks[i]->getPos(0) + max),
//                           (h / (max))*(max - landmarks[i]->getPos(2)),
//                           3, 3, pen, brush );
        double p0 = landmarks[i]->getPos(0);
        double p2 = landmarks[i]->getPos(2);
        scene->addEllipse( w/2 + 10*p0,
                           h/2 - 10*p2,
                           0.3, 0.3, pen, brush );
    }
}

void NaiveSLAM::test()
{
    vector<CvScalar> localPositions;
    CvMat *pos = cvCreateMat( 3, 1, CV_32F );
    vector<CvPoint> matches;
    vector<CvMat *> newFeatures;
    vector<CvScalar> newPositions;
    vector<int> classLabels;

    qDebug( "Testing Translation" );
    {
        localPositions.push_back( cvScalar( 1, 0, 0 ) );
        localPositions.push_back( cvScalar( 0, 1, 0 ) );
        localPositions.push_back( cvScalar( 0, 0, 1 ) );

        cvmSet( pos, 0, 0, 2 );
        cvmSet( pos, 1, 0, 0 );
        cvmSet( pos, 2, 0, 0 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );
        cvmSet( pos, 0, 0, 1 );
        cvmSet( pos, 1, 0, 1 );
        cvmSet( pos, 2, 0, 0 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );
        cvmSet( pos, 0, 0, 1 );
        cvmSet( pos, 1, 0, 0 );
        cvmSet( pos, 2, 0, 1 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );

        matches.push_back( cvPoint( 0, 0 ) );
        matches.push_back( cvPoint( 1, 1 ) );
        matches.push_back( cvPoint( 2, 2 ) );

        updatePosition( localPositions, matches );

        localPositions.clear();
        matches.clear();
        reset();
    }
    qDebug( "Testing Rotation" );
    {
        localPositions.push_back( cvScalar( 1, 0, 0 ) );
        localPositions.push_back( cvScalar( 0, 1, 0 ) );
        localPositions.push_back( cvScalar( 0, 0, 1 ) );

        cvmSet( pos, 0, 0, 0 );
        cvmSet( pos, 1, 0, 0 );
        cvmSet( pos, 2, 0, -1 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );
        cvmSet( pos, 0, 0, 0 );
        cvmSet( pos, 1, 0, 1 );
        cvmSet( pos, 2, 0, 0 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );
        cvmSet( pos, 0, 0, 1 );
        cvmSet( pos, 1, 0, 0 );
        cvmSet( pos, 2, 0, 0 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );

        matches.push_back( cvPoint( 0, 0 ) );
        matches.push_back( cvPoint( 1, 1 ) );
        matches.push_back( cvPoint( 2, 2 ) );

        updatePosition( localPositions, matches );

        localPositions.clear();
        matches.clear();
        reset();
    }
    qDebug( "Testing Translation And Rotation" );
    {
        localPositions.push_back( cvScalar( 1, 0, 0 ) );
        localPositions.push_back( cvScalar( 0, 1, 0 ) );
        localPositions.push_back( cvScalar( 0, 0, 1 ) );

        cvmSet( pos, 0, 0, 1 );
        cvmSet( pos, 1, 0, 0 );
        cvmSet( pos, 2, 0, -1 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );
        cvmSet( pos, 0, 0, 1 );
        cvmSet( pos, 1, 0, 1 );
        cvmSet( pos, 2, 0, 0 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );
        cvmSet( pos, 0, 0, 2 );
        cvmSet( pos, 1, 0, 0 );
        cvmSet( pos, 2, 0, 0 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );

        matches.push_back( cvPoint( 0, 0 ) );
        matches.push_back( cvPoint( 1, 1 ) );
        matches.push_back( cvPoint( 2, 2 ) );

        updatePosition( localPositions, matches );

        localPositions.clear();
        matches.clear();
        reset();
    }
    qDebug( "Testing Quaternions" );
    {
        // Conversion to rotation.
        Quaternion q1( 0.9239, 0, -0.3827, 0 );
        CvMat *R1 = q1.getRotation();
        printMatrix( R1 );
        Quaternion q2( 0.9239, 0, 0.3827, 0 );
        CvMat *R2 = q2.getRotation();
        printMatrix( R2 );

        // Conversion from rotation.
        q1 = Quaternion::fromRotation( R1 );
        qDebug( q1.toString().toStdString().c_str() );
        q2 = Quaternion::fromRotation( R2 );
        qDebug( q2.toString().toStdString().c_str() );

        // Rotation of vector.
        cvmSet( pos, 0, 0, 1 );
        cvmSet( pos, 1, 0, 0.5 );
        cvmSet( pos, 2, 0, -1 );
        Quaternion::rotate( q1, pos );
        printMatrix( pos );
        cvmSet( pos, 0, 0, 0 );
        cvmSet( pos, 1, 0, 1.5 );
        cvmSet( pos, 2, 0, 0.5 );
        Quaternion::rotate( q2, pos );
        printMatrix( pos );

        // Rotation of quaternion.
        Quaternion q3 = Quaternion::rotate( q1, q2 );
        qDebug( q3.toString().toStdString().c_str() );
        q3 = Quaternion::rotate( q2, q1 );
        qDebug( q3.toString().toStdString().c_str() );
        q1.rotate( q2 );
        qDebug( q1.toString().toStdString().c_str() );

        cvReleaseMat( &R1 );
        cvReleaseMat( &R2 );
    }
    qDebug( "Testing Landmarks" );
    {
        cvSetIdentity( Robservation, cvScalar( 0.09 ) );
        cvmSet( pos, 0, 0, 1.3631 );
        cvmSet( pos, 1, 0, -0.7990 );
        cvmSet( pos, 2, 0, 3.9361 );
        landmarks.push_back( new Landmark( pos, Robservation, 0, 0 ) );

        cvSetIdentity( Robservation, cvScalar( 0.1763 ) );
        cvmSet( currentTranslation, 0, 0, -0.1415 );
        cvmSet( currentTranslation, 1, 0, 0.3936 );
        cvmSet( currentTranslation, 2, 0, -0.3374 );
        currentRotation.w = 0.9968;
        currentRotation.x = 0.0190;
        currentRotation.y = 0.0382;
        currentRotation.z = -0.0674;

        newFeatures.push_back( cvCreateMat( 64, 1, CV_32F ) );
        newPositions.push_back( cvScalar( 1.3573, -0.7878, 3.9181 ) );
        classLabels.push_back( 0 );
        bool found[] = { true };
        matches.push_back( cvPoint( 0, 0 ) );

        updateMap( newFeatures, newPositions, found, matches, classLabels  );

        for ( int i = 0; i < (int)newFeatures.size(); i++ )
        {
            cvReleaseMat( &newFeatures[i] );
        }
        newFeatures.clear();
        newPositions.clear();
        matches.clear();
    }

    cvReleaseMat( &pos );
}

void NaiveSLAM::reset()
{
    for ( int i = 0; i < (int)landmarks.size(); i++ )
    {
        delete landmarks[i];
    }
    landmarks.clear();
    cvClearSeq( features );
    currentRotation = Quaternion();
    cvSetZero( currentTranslation );
    lastRotation = Quaternion();
    cvSetZero( lastTranslation );storage = cvCreateMemStorage( 0 );
    features = cvCreateSeq( 0, sizeof(CvSeq), 64*sizeof(float), storage );
    inited = false;
}

void NaiveSLAM::save( QTextStream &ts )
{
    // Store general information.
    ts << 1 << endl;                                                // number of particles
    ts << features->total << endl;                                  // number of features
    ts << currentRotation.w << " " << currentRotation.x << " "      // current rotation
       << currentRotation.y << " " << currentRotation.z << endl;
    ts << cvmGet( currentTranslation, 0, 0 ) << " "                 // current translation
       << cvmGet( currentTranslation, 1, 0 ) << " "
       << cvmGet( currentTranslation, 2, 0 ) << endl;
    ts << endl;

    // Store all features.
    for ( int i = 0; i < features->total; i++ )
    {
        double *feature = (double *)cvGetSeqElem( features, i );
        for ( int j = 0; j < 64 ; j++ )
        {
            ts << feature[j] << " ";
        }
        ts << endl;
    }
    ts << endl;

    // Store all particle filters.
    for ( int i = 0; i < 1; i++ )
    {
        // Store all landmarks.
        for ( int j = 0; j < features->total; j++ )
        {
            landmarks[j]->save( ts );
        }
        ts << endl;
    }
}

void NaiveSLAM::load( QString path )
{
    /*
    CvFileStorage *fileStorage = cvOpenFileStorage( fileName, storage, CV_STORAGE_READ );
    features = (CvSeq *)cvReadByName( fileStorage, NULL, "features" );
    positions = (CvSeq *)cvReadByName( fileStorage, NULL, "positions" );
    sigmas = (CvSeq *)cvReadByName( fileStorage, NULL, "sigmas" );
    classes = (CvSeq *)cvReadByName( fileStorage, NULL, "classes" );
    inited = false;
    cvReleaseFileStorage( &fileStorage );
    */
}
