#include "visualslamparticle.h"
#include <Module_VisualSLAM/helpers.h>
#include <QGraphicsTextItem>
#include <time.h>

#define DEFAULT_OBSERVATION_VARIANCE    0.09
#define DEFAULT_TRANSLATION_VARIANCE    0.09
#define DEFAULT_ROTATION_VARIANCE       0.01

VisualSLAMParticle::VisualSLAMParticle()
{
    currentTranslation = cvCreateMat( 3, 1, CV_32F );
    cvSetZero( currentTranslation );
    lastTranslation = cvCreateMat( 3, 1, CV_32F );
    cvSetZero( lastTranslation );

    // Initialize FastSLAM parameters.
    Robservation = cvCreateMat( 3, 3, CV_32F );
    cvSetIdentity( Robservation, cvScalar( DEFAULT_OBSERVATION_VARIANCE ) );

    Rstate = cvCreateMat( 7, 7, CV_32F );
    cvSetIdentity( Rstate, cvScalar( DEFAULT_ROTATION_VARIANCE ) );
    cvmSet( Rstate, 0, 0, DEFAULT_TRANSLATION_VARIANCE );
    cvmSet( Rstate, 1, 1, DEFAULT_TRANSLATION_VARIANCE );
    cvmSet( Rstate, 2, 2, DEFAULT_TRANSLATION_VARIANCE );

    Gobservation = cvCreateMat( 3, 3, CV_32F );
    Gstate = cvCreateMat( 3, 7, CV_32F );

    L = cvCreateMat( 7, 7, CV_32F );
    X = cvCreateMat( 7, 1, CV_32F );

    P3 = cvCreateMat( 3, 3, CV_32F );
    Q3 = cvCreateMat( 3, 3, CV_32F );
    K = cvCreateMat( 3, 3, CV_32F );
    R = cvCreateMat( 3, 3, CV_32F );
    U = cvCreateMat( 3, 3, CV_32F );
    D = cvCreateMat( 3, 3, CV_32F );
    V = cvCreateMat( 3, 3, CV_32F );
    meanLocalPosition = cvCreateMat( 3, 1, CV_32F );
    meanGlobalPosition = cvCreateMat( 3, 1, CV_32F );

    inited = false;
}

VisualSLAMParticle::VisualSLAMParticle( const VisualSLAMParticle *particle )
{
    currentTranslation = cvCreateMat( 3, 1, CV_32F );
    cvCopy( particle->currentTranslation, currentTranslation );
    lastTranslation = cvCreateMat( 3, 1, CV_32F );
    cvCopy( particle->lastTranslation, lastTranslation );

    currentRotation = particle->currentRotation;
    lastRotation = particle->lastRotation;

    landmarks = particle->landmarks;
    for ( int i = 0; i < (int)landmarks.size(); i++ )
    {
        landmarks[i]->addReference();
    }

    // Initialize FastSLAM parameters.
    Robservation = cvCreateMat( 3, 3, CV_32F );
    cvSetIdentity( Robservation, cvScalar( DEFAULT_OBSERVATION_VARIANCE ) );

    Rstate = cvCreateMat( 7, 7, CV_32F );
    cvSetIdentity( Rstate, cvScalar( DEFAULT_ROTATION_VARIANCE ) );
    cvmSet( Rstate, 0, 0, DEFAULT_TRANSLATION_VARIANCE );
    cvmSet( Rstate, 1, 1, DEFAULT_TRANSLATION_VARIANCE );
    cvmSet( Rstate, 2, 2, DEFAULT_TRANSLATION_VARIANCE );

    Gobservation = cvCreateMat( 3, 3, CV_32F );
    Gstate = cvCreateMat( 3, 7, CV_32F );

    L = cvCreateMat( 7, 7, CV_32F );
    X = cvCreateMat( 7, 1, CV_32F );

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

VisualSLAMParticle::~VisualSLAMParticle()
{
    cvReleaseMat( &currentTranslation );
    cvReleaseMat( &lastTranslation );
    cvReleaseMat( &Robservation );
    cvReleaseMat( &Rstate );
    cvReleaseMat( &Gobservation );
    cvReleaseMat( &Gstate );
    cvReleaseMat( &L );
    cvReleaseMat( &X );
    cvReleaseMat( &P3 );
    cvReleaseMat( &Q3 );
    cvReleaseMat( &K );
    cvReleaseMat( &R );
    cvReleaseMat( &U );
    cvReleaseMat( &D );
    cvReleaseMat( &V );
    cvReleaseMat( &meanLocalPosition );
    cvReleaseMat( &meanGlobalPosition );

    int references = 0;
    for ( int i = 0; i < (int)landmarks.size(); i++ )
    {
        references = landmarks[i]->removeReference();
        if ( references < 1 )
        {
            delete( landmarks[i] );
        }
    }
}

double VisualSLAMParticle::update( vector<CvScalar> pos3D, vector<CvPoint> mapMatches, bool *found )
{
    int totalFeatures = (int)pos3D.size();
    double score = 1.0;

    if ( totalFeatures > 0 )
    {
        // Determine current Position from the matched features. This will
        // only work if there are at least 3 matches.
        if ( mapMatches.size() >= 3 )
        {
            updatePosition( pos3D, mapMatches );
        }

        // Update the map with the new Features.
        score = updateMap( pos3D, found, mapMatches );
    }

    return score;
}

bool VisualSLAMParticle::updatePosition( vector<CvScalar> newPositions, vector<CvPoint> matches )
{
    bool success = true;

    lastRotation = currentRotation;
    cvCopy( currentTranslation, lastTranslation );

    int numPositions = (int)newPositions.size();
    bool *maxConsensusSet = new bool[numPositions];
    bool *tempConsensusSet = new bool[numPositions];
    int maxConsensus = 0;
    int tempMaxConsensus = 0;
    for ( int i = 0; i < 20; i++ )
    {
        // Calculate position with three random samples.
        memset( tempConsensusSet, false, numPositions*sizeof(bool) );

        drawRandomSamples( numPositions, tempConsensusSet );
        calcPosition( newPositions, matches, tempConsensusSet, 3 );

        // Check which samples are inside the threshold.
        errorGreaterT( newPositions, matches, 0.3, tempConsensusSet, tempMaxConsensus );

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
    if ( maxConsensus >= 3 )
    {
        calcPosition( newPositions, matches, maxConsensusSet, maxConsensus );

        if ( inited )
        {
            // Check if the calculated translation and rotation seem sensible.
            Quaternion diffRotation = lastRotation - currentRotation;
            if ( cvNorm( lastTranslation, currentTranslation ) > 2.0 ||
                 diffRotation.norm() > 0.7 )
            {
                success = false;
                qDebug( "No SUCCESS: R = %f, t = %f", diffRotation.norm(), cvNorm( lastTranslation, currentTranslation ) );
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

    delete( tempConsensusSet );
    delete( maxConsensusSet );

    return success;
}

void VisualSLAMParticle::drawRandomSamples( int num, bool *selected )
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

void VisualSLAMParticle::calcPosition( vector<CvScalar> newPositions, vector<CvPoint> matches, bool *selected, int num )
{
    cvZero( meanLocalPosition );
    cvZero( meanGlobalPosition );
    CvMat *P;
    CvMat *Q;

    if ( num == 3 )
    {
        P = P3;
        cvSetZero( P );
        Q = Q3;
    }
    else
    {
        P = cvCreateMat( 3, num, CV_32F );
        Q = cvCreateMat( 3, num, CV_32F );
    }
    cvSetZero( P );
    cvSetZero( Q );

    int k = 0;
    for ( int i = 0; i < (int) matches.size(); i++ )
    {
        if ( selected[i] )
        {
            for ( int j = 0; j < 3; j++ )
            {
                cvmSet( P, j, k, (float)newPositions[matches[i].x].val[j] );
                cvmSet( Q, j, k, (float)landmarks[matches[i].y]->getPos(j) );

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
    cvGEMM( Q, P, 1, NULL, 1, K, CV_GEMM_B_T );

//    qDebug( "SVD start" );
//    printMatrix( K, "K" );
//    printMatrix( Q, "Q" );
//    printMatrix( P, "P" );
//    printMatrix( meanLocalPosition, "meanLocalPosition" );
//    printMatrix( meanGlobalPosition, "meanGlobalPosition" );
    cvSVD( K, D, U, V, CV_SVD_MODIFY_A | CV_SVD_V_T );
//    qDebug( "SVD end" );

    cvSetIdentity( D, cvScalar(1) );
    cvMatMul( U, D, R );
    cvMatMul( R, V, R );

    cvMatMul( R, meanLocalPosition, currentTranslation );
    cvSub( meanGlobalPosition, currentTranslation, currentTranslation );

    currentRotation = Quaternion::fromRotation( R );

    if ( num != 3 )
    {
        cvReleaseMat( &P );
        cvReleaseMat( &Q );
    }
}

void VisualSLAMParticle::errorGreaterT( vector<CvScalar> newPositions, vector<CvPoint> matches, double T, bool *check, int &count )
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
        Quaternion::rotate( currentRotation, tempLocalPosition );
        cvAdd( tempLocalPosition, currentTranslation, tempLocalPosition );

        if ( cvNorm( tempLocalPosition, tempGlobalPosition ) <= T )
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

double VisualSLAMParticle::updateMap( vector<CvScalar> newPositions, bool *found, vector<CvPoint> matches )
{
    confidence = 1.0;

    CvMat *newPosition = cvCreateMat( 3, 1, CV_32F );
    CvMat *expectedPosition = cvCreateMat( 3, 1, CV_32F );
    CvMat *R = currentRotation.getRotation();
    int oldLandmarkCount = (int)landmarks.size();
    bool *toDelete = new bool[oldLandmarkCount];
    memset( toDelete, false, oldLandmarkCount * sizeof(bool) );

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

    // Choose random position according to given covariance.
    cholesky( Sigma, L );
    CvRNG rng = cvRNG( cvGetTickCount() );
    cvRandArr( &rng, X, CV_RAND_NORMAL, cvRealScalar( 0 ), cvRealScalar( 1 ) );
    cvGEMM( L, X, 1, sfilt, 1, sfilt );

    cvmSet( currentTranslation, 0, 0, cvmGet( sfilt, 0, 0 ) );
    cvmSet( currentTranslation, 1, 0, cvmGet( sfilt, 1, 0 ) );
    cvmSet( currentTranslation, 2, 0, cvmGet( sfilt, 2, 0 ) );
    currentRotation.w = cvmGet( sfilt, 3, 0 );
    currentRotation.x = cvmGet( sfilt, 4, 0 );
    currentRotation.y = cvmGet( sfilt, 5, 0 );
    currentRotation.z = cvmGet( sfilt, 6, 0 );
    currentRotation.normalize();

    cvReleaseMat( &Sigma );
    cvReleaseMat( &Sigmainv );
    cvReleaseMat( &sfilt );
    cvReleaseMat( &Z );
    cvReleaseMat( &Zinv );
    cvReleaseMat( &Mtemp1 );
    cvReleaseMat( &diffPos );

    for ( int i = 0; i < (int)matches.size(); i++ )
    {
        cvmSet( newPosition, 0, 0, newPositions[matches[i].x].val[0] );
        cvmSet( newPosition, 1, 0, newPositions[matches[i].x].val[1] );
        cvmSet( newPosition, 2, 0, newPositions[matches[i].x].val[2] );

        cvSub( landmarks[matches[i].y]->getPos(), currentTranslation, expectedPosition );
        cvGEMM( R, expectedPosition, 1, NULL, 1, expectedPosition, CV_GEMM_A_T );

        getObservationJacobian( Gobservation );
        getStateJacobian( matches[i].y, Gstate );

        // Check if this landmark belongs only to this particle. If there are
        // more particles referencing the landmark, create a copy.
        if ( landmarks[matches[i].y]->references > 1 )
        {
            landmarks[matches[i].y]->references--;
            Landmark *copy = new Landmark( *landmarks[matches[i].y] );
            copy->references = 1;
            landmarks[matches[i].y] = copy;
        }

        landmarks[matches[i].y]->update( newPosition, expectedPosition,
                                         Robservation, Rstate,
                                         Gobservation, Gstate );
    }

    for ( int i = 0; i < (int)newPositions.size(); i++ )
    {
        if ( !found[i] )
        {
            // Calculate global position of the feature.
            cvmSet( newPosition, 0, 0, newPositions[i].val[0] );
            cvmSet( newPosition, 1, 0, newPositions[i].val[1] );
            cvmSet( newPosition, 2, 0, newPositions[i].val[2] );
            cvMatMulAdd( R, newPosition, currentTranslation, newPosition );

            landmarks.push_back( new Landmark( newPosition, Robservation, (int)landmarks.size(), 0 ) );
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

    return confidence;
}

void VisualSLAMParticle::getObservationJacobian( CvMat *Gobservation )
{
    CvMat *R = currentRotation.getRotation();
    cvCopy( R, Gobservation );
    cvTranspose( Gobservation, Gobservation );
    cvReleaseMat( &R );
}

void VisualSLAMParticle::getStateJacobian( int landmarkNr, CvMat *Gstate )
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

void VisualSLAMParticle::plot( QGraphicsScene *scene )
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

        double p0 = landmarks[i]->getPos(0);
        double p2 = landmarks[i]->getPos(2);
        scene->addEllipse( w/2 + 10*p0,
                           h/2 - 10*p2,
                           0.3, 0.3, pen, brush );
    }
}

void VisualSLAMParticle::save( QTextStream &ts )
{
    ts << currentRotation.w << " " << currentRotation.x << " "      // current rotation
       << currentRotation.y << " " << currentRotation.z << endl;
    ts << cvmGet( currentTranslation, 0, 0 ) << " "                 // current translation
       << cvmGet( currentTranslation, 1, 0 ) << " "
       << cvmGet( currentTranslation, 2, 0 ) << endl;
    ts << endl;

    // Store all landmarks.
    for ( int j = 0; j < (int)landmarks.size(); j++ )
    {
        landmarks[j]->save( ts );
    }
    ts << endl;
}

void VisualSLAMParticle::load( QTextStream &ts, int landmarkCount )
{
    double temp;

    // Load last rotation.
    ts >> currentRotation.w >> currentRotation.x >> currentRotation.y >> currentRotation.z;

    // Load last translation.
    ts >> temp;
    cvmSet( currentTranslation, 0, 0, temp );
    ts >> temp;
    cvmSet( currentTranslation, 1, 0, temp );
    ts >> temp;
    cvmSet( currentTranslation, 2, 0, temp );

    // Delete all landmarks.
    for ( int j = 0; j < (int)landmarks.size(); j++ )
    {
        delete( landmarks[j] );
    }
    landmarks.clear();

    // Read landmarks from file.
    for ( int j = 0; j < landmarkCount; j++ )
    {
        Landmark *l = new Landmark();
        l->load( ts );
    }
}
