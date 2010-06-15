#include "naiveslam.h"
#include <Module_VisualSLAM/feature/feature.h>
#include <Module_VisualSLAM/helpers.h>
#include <iostream>
#include <QGraphicsTextItem>

NaiveSLAM::NaiveSLAM( int particleCount )
{
    // Initialize feature sequence.
    storage = cvCreateMemStorage( 0 );
    features = cvCreateSeq( 0, sizeof(CvSeq), 64*sizeof(float), storage );

    // Initialize particles.
    for ( int i = 0; i < particleCount; i++ )
    {
        particles.push_back( new VisualSLAMParticle() );
    }
}

NaiveSLAM::~NaiveSLAM()
{
    cvReleaseMemStorage( &storage );
    for ( int i = 0; i < (int)particles.size(); i++ )
    {
        delete( particles[i] );
    }
}

bool NaiveSLAM::update( vector<CvMat *> descriptor, vector<CvScalar> pos3D, vector<int> classLabels )
{
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

        // Add new features.
        for ( int i = 0; i < totalFeatures; i++ )
        {
            if ( !found[i] )
            {
                cvSeqPush( features, descriptor[i]->data.fl );
            }
        }

        // Add class for each new feature.
        for ( int i = 0; i < (int)mapMatches.size(); i++ )
        {
            classes.push_back( classLabels[mapMatches[i].x] );
        }

        // Update all particles and store weights. Also calculate best current particle.
        double *weights = new double[(int)particles.size()];
        bestParticle = -1;
        double bestParticleWeight = -1;
        for ( int i = 0; i < (int)particles.size(); i++ )
        {
            weights[i] = particles[i]->update( pos3D, mapMatches, found );
            if ( weights[i] > bestParticleWeight )
            {
                bestParticle = i;
                bestParticleWeight = weights[i];
            }
        }

        // Resample particles.
        resampleParticles( weights );

        // Calculate position from best particle.
        float yaw;
        float pitch;
        float roll;
        particles[bestParticle]->currentRotation.getYawPitchRoll( yaw, pitch, roll );
        double x = cvmGet( particles[bestParticle]->currentTranslation, 0, 0 );
        double y = cvmGet( particles[bestParticle]->currentTranslation, 1, 0 );
        double z = cvmGet( particles[bestParticle]->currentTranslation, 2, 0 );
        pos = Position( x, y, z, roll, pitch, yaw );

        delete( found );
    }

    return success;
}

void NaiveSLAM::resampleParticles( double *weights )
{
    int M = (int)particles.size();
    double sum = 0.0;
    pos.setX( 0 );
    pos.setY( 0 );
    pos.setZ( 0 );

    // Normalize weights.
    for ( int i = 0; i < M; i++ )
    {
        sum += weights[i];
    }

    // Calculate cumulative sum. (and weighted position)
    weights[0] = weights[0] / sum;
    for ( int i = 1; i < M; i++ )
    {
        pos.setX( pos.getX() + (weights[i] / sum) * cvmGet( particles[i]->currentTranslation, 0, 0 ) );
        pos.setY( pos.getY() + (weights[i] / sum) * cvmGet( particles[i]->currentTranslation, 1, 0 ) );
        pos.setZ( pos.getZ() + (weights[i] / sum) * cvmGet( particles[i]->currentTranslation, 2, 0 ) );
        weights[i] = weights[i-1] + weights[i] / sum;
    }

    // Select M random particles.
    CvRNG rng = cvRNG( cvGetTickCount() );
    CvMat *r = cvCreateMat( 1, M, CV_32F );
    cvRandArr( &rng, r, CV_RAND_UNI, cvRealScalar( 0 ), cvRealScalar( 1 ) );
    for ( int i = 0; i < M; i++ )
    {
        for ( int j = 0; j < M; j++ )
        {
            if ( weights[j] >= cvmGet( r, 0, i ) )
            {
//                qDebug( "%d", j );
                particles.push_back( new VisualSLAMParticle( particles[j] ) );
                break;
            }
        }
    }

    // Delete old particles.
    for ( int i = 0; i < M; i++ )
    {
        delete( particles[0] );
        particles.erase( particles.begin() );
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
    // Plot all particle positions.
    double w = scene->width();
    double h = scene->height();

    QPen pen(Qt::gray);
    QBrush brush(Qt::gray);
    pen.setWidth( 1.0 );
    for ( int i = 0; i < (int)particles.size(); i++ )
    {
        double pos1 = cvmGet( particles[i]->currentTranslation, 0, 0 );
        double pos2 = -cvmGet( particles[i]->currentTranslation, 2, 0 );
//        CvMat *R = currentRotation.getRotation();
//        double z1 = cvmGet( R, 2, 0 );
//        double z2 = cvmGet( R, 2, 2 );
//        cvReleaseMat( &R );
        scene->addEllipse( w/2 + 10*pos1 - 0.25,
                           h/2 + 10*pos2 - 0.25,
                           0.5, 0.5, pen, brush );
//        scene->addLine( w/2 + 10*pos1,
//                        h/2 + 10*pos2,
//                        w/2 + 10*(pos1 - z1),
//                        h/2 + 10*(pos2 - z2),
//                        pen );
    }

    // Plot mean position.
    pen = QPen(Qt::green);
    brush = QBrush(Qt::green);
    scene->addEllipse( w/2 + 10*pos.getX() - 0.5,
                       h/2 - 10*pos.getZ() - 0.5,
                       1.0, 1.0, pen, brush );

    // Plot best particle map.
    if ( bestParticle >= 0 && bestParticle < (int)particles.size() )
    {
        particles[bestParticle]->plot( scene );
    }
}

void NaiveSLAM::reset()
{
    for ( int i = 0; i < (int)particles.size(); i++ )
    {
        delete( particles[i] );
    }
    for ( int i = 0; i < (int)particles.size(); i++ )
    {
        particles[i] = new VisualSLAMParticle();
    }
    cvClearSeq( features );
    classes.clear();
}

void NaiveSLAM::save( QTextStream &ts )
{
    // Store general information.
    ts << (int)particles.size() << endl;                            // number of particles
    ts << features->total << endl;                                  // number of features

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
    for ( int i = 0; i < (int)particles.size(); i++ )
    {
        particles[i]->save( ts );
    }
}

void NaiveSLAM::load( QTextStream &ts )
{
    // Load number of particles and features.
    int particleCount;
    int featureCount;
    ts >> particleCount >> featureCount;

    // Load features.
    cvClearSeq( features );
    double feature[64];
    for ( int i = 0; i < featureCount; i++ )
    {
        for ( int j = 0; j < 64 ; j++ )
        {
            ts >> feature[j];
        }
        cvSeqPush( features, feature );
    }

    // Load all particles.
    for ( int i = 0; i < (int)particles.size(); i++ )
    {
        delete( particles[i] );
    }
    particles.clear();
    for ( int i = 0; i < particleCount; i++ )
    {
        VisualSLAMParticle *particle = new VisualSLAMParticle();
        particle->load( ts, featureCount );
        particles.push_back( particle );
    }
}

/*
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
*/
