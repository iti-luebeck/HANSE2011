#include "landmark.h"
#include "../helpers.h"

Landmark::Landmark( CvMat *observation, CvMat *Robservation, int featureNr, int classNr )
{
    references = 0;
    this->featureNr = featureNr;
    this->classNr = classNr;
    pos = cvCreateMat( 3, 1, CV_32F );
    Sigma = cvCreateMat( 3, 3, CV_32F );
    Z = cvCreateMat( 3, 3, CV_32F );
    Zinv = cvCreateMat( 3, 3, CV_32F );
    K = cvCreateMat( 3, 3, CV_32F );
    diffPos = cvCreateMat( 3, 1, CV_32F );
    I3 = cvCreateMat( 3, 3, CV_32F );
    cvSetIdentity( I3, cvScalar( 1 ) );
    Mtemp1 = cvCreateMat( 3, 3, CV_32F );
    Mtemp2 = cvCreateMat( 3, 7, CV_32F );
    Mtemp3 = cvCreateMat( 3, 3, CV_32F );
    Mtemp4 = cvCreateMat( 3, 3, CV_32F );
    L = cvCreateMat( 3, 3, CV_32F );

    cvCopy( observation, pos );
    cvCopy( Robservation, Sigma );
}

Landmark::~Landmark()
{
    cvReleaseMat( &pos );
    cvReleaseMat( &Sigma );
    cvReleaseMat( &Z );
    cvReleaseMat( &Zinv );
    cvReleaseMat( &K );
    cvReleaseMat( &diffPos );
    cvReleaseMat( &I3 );
    cvReleaseMat( &Mtemp1 );
    cvReleaseMat( &Mtemp2 );
    cvReleaseMat( &Mtemp3 );
    cvReleaseMat( &Mtemp4 );
    cvReleaseMat( &L );
}

double Landmark::update( CvMat *observation, CvMat *expectedObservation,
                         CvMat *Robservation, CvMat *Rstate,
                         CvMat *Gobservation, CvMat *Gstate )
{
//    qDebug( "observation" );
//    printMatrix( observation );
//    qDebug( "expectedObservation" );
//    printMatrix( expectedObservation );
//    qDebug( "Robservation" );
//    printMatrix( Robservation );
//    qDebug( "Rstate" );
//    printMatrix( Rstate );
//    qDebug( "Gobservation" );
//    printMatrix( Gobservation );
//    qDebug( "Gstate" );
//    printMatrix( Gstate );
//    qDebug( "pos(%d) before", featureNr );
//    printMatrix( pos );
//    qDebug( "Sigma" );
//    printMatrix( Sigma );

    // Z = Roprime + Gtheta * obj.sigma(:,(3*matches(i)-2):(3*matches(i))) * Gtheta';
    cvGEMM( Sigma, Gobservation, 1, NULL, 1, Z, CV_GEMM_B_T );
    cvGEMM( Gobservation, Z, 1, Robservation, 1, Z );
//    qDebug( "Z" );
//    printMatrix( Z );

    // Calculate Kalman gain.
    // K = obj.sigma(:,(3*matches(i)-2):(3*matches(i))) * Gtheta' * inv(Z);
    cvGEMM( Sigma, Gobservation, 1, NULL, 1, K, CV_GEMM_B_T );
    cvInv( Z, Zinv, CV_SVD );
    cvGEMM( K, Zinv, 1, NULL, 1, K );
//    qDebug( "K" );
//    printMatrix( K );

    // Calculate new position.
    // obj.mu(:,matches(i)) = obj.mu(:,matches(i)) + K * (z(:,i) - zhat);
    cvSub( observation, expectedObservation, diffPos );
    cvGEMM( K, diffPos, 1, pos, 1, pos );
//    qDebug( "pos(%d) after", featureNr );
//    printMatrix( pos );

    // Calculate new covariance matrix.
    // obj.sigma(:,(3*matches(i)-2):(3*matches(i))) = (eye(3) - K * Gtheta) * obj.sigma(:,(3*matches(i)-2):(3*matches(i)));
    cvGEMM( K, Gobservation, -1, I3, 1, Mtemp1 );
    cvGEMM( Mtemp1, Sigma, 1, NULL, 1, Sigma );
//    qDebug( "Sigma" );
//    printMatrix( Sigma );

    // L = (Gs * Rt * Gs') + (Gtheta * obj.sigma(:,(3*matches(i)-2):(3*matches(i))) * Gtheta') + Roprime;
    cvGEMM( Gstate, Rstate, 1, NULL, 1, Mtemp2 );
    cvGEMM( Mtemp2, Gstate, 1, NULL, 1, Mtemp4, CV_GEMM_B_T );
    cvGEMM( Gobservation, Sigma, 1, NULL, 1, Mtemp3 );
    cvGEMM( Mtemp3, Gobservation, 1, Robservation, 1, Mtemp3, CV_GEMM_B_T );
    cvAdd( Mtemp4, Mtemp3, L );
//    qDebug( "L" );
//    printMatrix( L );

    // pi = mvnpdf(z(:,i), zhat, L);
    double p = mvnpdf( diffPos, NULL, L );
//    qDebug( "p = %f", p );
    return p;
}

int Landmark::removeReference()
{
    return --references;
}

void Landmark::addReference()
{
    references++;
}

CvMat *Landmark::getPos()
{
    return pos;
}

double Landmark::getPos( int i )
{
    if ( i < 0 || i > 2 )
    {
        return 0;
    }
    else
    {
        return cvmGet( pos, i, 0 );
    }
}

int Landmark::getClass()
{
    return classNr;
}

CvMat *Landmark::getSigma()
{
    return Sigma;
}
