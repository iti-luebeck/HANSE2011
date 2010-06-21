#include "odometry.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include "../feature/feature.h"

Odometry::Odometry()
{
    lastDescriptors = NULL;
}

Odometry::~Odometry()
{
    if ( lastDescriptors ) cvReleaseMat( &lastDescriptors );
}

void Odometry::getOdometry( CvMat *descriptors, vector<CvScalar>newPositions2D, vector<CvScalar>newPositions3D, CvMat *R, CvMat *T )
{
    // Find matches between last and current image.
    vector<CvScalar> lastMatches2D;
    vector<CvScalar> nextMatches2D;
    vector<CvPoint> matches;
    Feature::matchFeatures( descriptors, lastDescriptors, matches );
    for ( int i = 0; i < (int)matches.size(); i++ )
    {
        nextMatches2D.push_back( newPositions2D[matches[i].x] );
        lastMatches2D.push_back( last2DPositions[matches[i].y] );
    }

    // Find best essential matrix using RANSAC.
    CvMat *E = cvCreateMat( 3, 3, CV_32F );
    cvSetIdentity( E, cvScalar( 1 ) );
    CvMat *tempE = cvCreateMat( 3, 3, CV_32F );
    int maxConsensus = 0;
    for ( int k = 0; k < 1000; k++ )
    {
        vector<CvMat *> Es;
        calcFivePoint( lastMatches2D, nextMatches2D, Es );
        int tempConsensus = calcConsensusSet( lastMatches2D, nextMatches2D, Es, tempE );
        if ( tempConsensus > maxConsensus )
        {
            cvCopy( tempE, E );
            maxConsensus = tempConsensus;
        }

        // Release potential matrices.
        for ( int i = 0; i < (int)Es.size(); i++ )
        {
            cvReleaseMat( &Es[i] );
        }
        Es.clear();
    }

    // Find possible camera matrices.
    vector<CvMat *> Rs;
    vector<CvMat *> Ts;
    getCameraMatrices( E, Rs, Ts );

    // Get correct camera matrix.
    getCorrectCameraMatrix( Rs, Ts, R, T, lastMatches2D[0], nextMatches2D[0] );

    cvReleaseMat( &Rs[0] );
    cvReleaseMat( &Rs[1] );
    Rs.clear();
    cvReleaseMat( &Ts[0] );
    cvReleaseMat( &Ts[1] );
    Ts.clear();

    cvReleaseMat( &E );
    cvReleaseMat( &tempE );

    // Replace descriptors and positions.
    if ( lastDescriptors ) cvReleaseMat( &lastDescriptors );
    lastDescriptors = cvCreateMat( descriptors->rows, descriptors->cols, CV_32F );
    cvCopy( descriptors, lastDescriptors );
    last2DPositions.clear();
    for ( int i = 0; i < (int)newPositions2D.size(); i++ )
    {
        last2DPositions.push_back( newPositions2D[i] );
    }
    last3DPositions.clear();
    for ( int i = 0; i < (int)newPositions3D.size(); i++ )
    {
        last3DPositions.push_back( newPositions3D[i] );
    }
}

int Odometry::calcConsensusSet( vector<CvScalar>last, vector<CvScalar>next, vector<CvMat *> Es, CvMat *E )
{
    int consensus = -1;
    CvMat *q1 = cvCreateMat( 3, 1, CV_32F );
    cvmSet( q1, 2, 0, 1.0 );
    CvMat *q2 = cvCreateMat( 3, 1, CV_32F );
    cvmSet( q2, 2, 0, 1.0 );
    for ( int k = 0; k < (int)Es.size(); k++ )
    {
        int tempConsensus = 0;
        for ( int i = 0; i < (int)last.size(); i++ )
        {
            cvmSet( q1, 0, 0, last[i].val[0] );
            cvmSet( q1, 1, 0, last[i].val[1] );
            cvmSet( q2, 0, 0, next[i].val[0] );
            cvmSet( q2, 1, 0, next[i].val[1] );
            cvMatMul( Es[k], q2, q2 );
            double d = cvDotProduct( q1, q2 );
            if ( d < 0.0001 )
            {
                tempConsensus++;
            }
        }
        if ( tempConsensus > consensus )
        {
            cvCopy( Es[k], E );
            consensus = tempConsensus;
        }
    }
    cvReleaseMat( &q1 );
    cvReleaseMat( &q2 );
    return consensus;
}

void Odometry::calcFivePoint( vector<CvScalar> last, vector<CvScalar> next, vector<CvMat *> &Es )
{
    int numTotal = (int)last.size();
    CvMat *P = cvCreateMat( 3, 5, CV_64F );
    CvMat *Q = cvCreateMat( 3, 5, CV_64F );
    CvMat *M = cvCreateMat( 5, 9, CV_64F );
    CvMat *EE = cvCreateMat( 9, 4, CV_64F );
    int k = 0;
    bool *selected = new bool[numTotal];
    memset( selected, false, sizeof(bool) );
    drawRandomSamples( 5, numTotal, selected );
    for ( int i = 0; i < numTotal; i++ )
    {
        if ( selected[i] )
        {
            cvmSet( P, 0, k, next[i].val[0] );
            cvmSet( P, 1, k, next[i].val[1] );
            cvmSet( P, 2, k, next[i].val[2] );
            cvmSet( Q, 0, k, last[i].val[0] );
            cvmSet( Q, 1, k, last[i].val[1] );
            cvmSet( Q, 2, k, last[i].val[2] );
        }
    }
    for ( int i = 0; i < 5; i++ )
    {
        cvmSet( M, i, 0, cvmGet( P, 0, i ) * cvmGet( Q, 0, i ) );
        cvmSet( M, i, 0, cvmGet( P, 1, i ) * cvmGet( Q, 0, i ) );
        cvmSet( M, i, 0, cvmGet( P, 2, i ) * cvmGet( Q, 0, i ) );
        cvmSet( M, i, 0, cvmGet( P, 0, i ) * cvmGet( Q, 1, i ) );
        cvmSet( M, i, 0, cvmGet( P, 1, i ) * cvmGet( Q, 1, i ) );
        cvmSet( M, i, 0, cvmGet( P, 2, i ) * cvmGet( Q, 1, i ) );
        cvmSet( M, i, 0, cvmGet( P, 0, i ) * cvmGet( Q, 2, i ) );
        cvmSet( M, i, 0, cvmGet( P, 1, i ) * cvmGet( Q, 2, i ) );
        cvmSet( M, i, 0, cvmGet( P, 2, i ) * cvmGet( Q, 2, i ) );
    }

    CvMat *D = cvCreateMat( 5, 9, CV_64F );
    CvMat *V = cvCreateMat( 9, 9, CV_64F );
    cvSVD( M, D, NULL, V );
    for ( int i = 0; i < 9; i++ )
    {
        cvmSet( EE, i, 0, cvmGet( V, i, 5 ) );
        cvmSet( EE, i, 1, cvmGet( V, i, 6 ) );
        cvmSet( EE, i, 2, cvmGet( V, i, 7 ) );
        cvmSet( EE, i, 3, cvmGet( V, i, 8 ) );
    }

    CvMat *A = cvCreateMat( 10, 20, CV_64F );
    fivePointHelper( EE, A );

    CvMat *A1 = cvCreateMat( 10, 10, CV_64F );
    CvMat *A2 = cvCreateMat( 10, 10, CV_64F );
    for ( int i = 0; i < 10; i++ )
    {
        for ( int j = 0; j < 10; j++ )
        {
            cvmSet( A1, i, j, cvmGet( A, i, j ) );
            cvmSet( A2, i, j, cvmGet( A, i, j + 10 ) );
        }
    }

    CvMat *A3 = cvCreateMat( 10, 10, CV_64F );
    cvSolve( A1, A2, A3, CV_SVD );

    CvMat *M1 = cvCreateMat( 10, 10, CV_64F );
    cvSetZero( M1 );
    for ( int j = 0; j < 10; j++ )
    {
        cvmSet( M1, 0, j, -cvmGet( A3, 0, j ) );
        cvmSet( M1, 1, j, -cvmGet( A3, 1, j ) );
        cvmSet( M1, 2, j, -cvmGet( A3, 2, j ) );
        cvmSet( M1, 3, j, -cvmGet( A3, 4, j ) );
        cvmSet( M1, 4, j, -cvmGet( A3, 5, j ) );
        cvmSet( M1, 5, j, -cvmGet( A3, 7, j ) );
    }
    cvmSet( M1, 6, 0, 1 );
    cvmSet( M1, 7, 1, 1 );
    cvmSet( M1, 8, 3, 1 );
    cvmSet( M1, 9, 6, 1 );

    gsl_matrix_view m = gsl_matrix_view_array( M1->data.db, 4, 4 );

    gsl_vector_complex *eval = gsl_vector_complex_alloc( 4 );
    gsl_matrix_complex *evec = gsl_matrix_complex_alloc( 4, 4 );

    gsl_eigen_nonsymmv_workspace *w = gsl_eigen_nonsymmv_alloc( 4 );
    gsl_eigen_nonsymmv( &m.matrix, eval, evec, w );
    gsl_eigen_nonsymmv_free( w );
    gsl_eigen_nonsymmv_sort( eval, evec, GSL_EIGEN_SORT_ABS_DESC );

    CvMat *SOLS = cvCreateMat( 4, 10, CV_64F );
    bool isReal[10];
    memset( isReal, true, 10*sizeof(bool) );
    for (int i = 0; i < 10; i++)
    {
        gsl_vector_complex_view evec_i = gsl_matrix_complex_column( evec, i );

        gsl_complex z1 = gsl_vector_complex_get(&evec_i.vector, 6);
        gsl_complex z2 = gsl_vector_complex_get(&evec_i.vector, 7);
        gsl_complex z3 = gsl_vector_complex_get(&evec_i.vector, 8);
        gsl_complex z4 = gsl_vector_complex_get(&evec_i.vector, 9);
        isReal[i] = (GSL_IMAG(z1) < 0.001) && (GSL_IMAG(z2) < 0.001) &&
                    (GSL_IMAG(z3) < 0.001) && (GSL_IMAG(z4) < 0.001);
        cvmSet( SOLS, 0, i, GSL_REAL(z1) / GSL_REAL(z4) );
        cvmSet( SOLS, 1, i, GSL_REAL(z2) / GSL_REAL(z4) );
        cvmSet( SOLS, 2, i, GSL_REAL(z3) / GSL_REAL(z4) );
        cvmSet( SOLS, 3, i, 1.0 );
    }

    gsl_vector_complex_free(eval);
    gsl_matrix_complex_free(evec);

    CvMat *Evec = cvCreateMat( 9, 10, CV_64F );
    cvMatMul( EE, SOLS, Evec );
    int nReal = 0;
    for ( int j = 0; j < 10; j++ )
    {
        double sum = 0;
        for ( int i = 0; i < 9; i++ )
        {
            sum += cvmGet( Evec, i, j )*cvmGet( Evec, i, j );
        }
        if ( sum > 0 )
        {
            for ( int i = 0; i < 9; i++ )
            {
                cvmSet( Evec, i, j, cvmGet( Evec, i, j ) / sqrt(sum) );
            }
        }
        else
        {
            isReal[j] = false;
        }
        if ( isReal[j] )
        {
            nReal++;
        }
    }

    for ( k = 0; k < 10; k++ )
    {
        if ( isReal[k] )
        {
            CvMat *E = cvCreateMat( 3, 3, CV_32F );
            for ( int i = 0; i < 3; i++ )
            {
                for ( int j = 0; j < 3; j++ )
                {
                    cvmSet( E, i, j, cvmGet( Evec, i*3 + j, k ) );
                }
            }
            Es.push_back( E );
        }
    }

    cvReleaseMat( &P );
    cvReleaseMat( &Q );
    cvReleaseMat( &M );
    cvReleaseMat( &EE );
    cvReleaseMat( &D );
    cvReleaseMat( &V );
    cvReleaseMat( &A );
    cvReleaseMat( &A1 );
    cvReleaseMat( &A2 );
    cvReleaseMat( &A3 );
    cvReleaseMat( &M1 );
}

void Odometry::fivePointHelper( CvMat *EEMat, CvMat *AMat )
{
    double EE[40];
    double A[200];

    double e00,e01,e02,e03,e04,e05,e06,e07,e08;
    double e10,e11,e12,e13,e14,e15,e16,e17,e18;
    double e20,e21,e22,e23,e24,e25,e26,e27,e28;
    double e30,e31,e32,e33,e34,e35,e36,e37,e38;

    double e002,e012,e022,e032,e042,e052,e062,e072,e082;
    double e102,e112,e122,e132,e142,e152,e162,e172,e182;
    double e202,e212,e222,e232,e242,e252,e262,e272,e282;
    double e302,e312,e322,e332,e342,e352,e362,e372,e382;

    double e003,e013,e023,e033,e043,e053,e063,e073,e083;
    double e103,e113,e123,e133,e143,e153,e163,e173,e183;
    double e203,e213,e223,e233,e243,e253,e263,e273,e283;
    double e303,e313,e323,e333,e343,e353,e363,e373,e383;

    // Transform matrix EE to vector.
    for ( int i = 0; i < 9; i++ )
    {
        for ( int j = 0; j < 4; j++ )
        {
            EE[j*9 + i] = cvmGet( EEMat, i, j );
        }
    }

    // Extract important matrix elements.
    {
        e00 = EE[0*9 + 0 ];
        e10 = EE[1*9 + 0 ];
        e20 = EE[2*9 + 0 ];
        e30 = EE[3*9 + 0 ];
        e01 = EE[0*9 + 1 ];
        e11 = EE[1*9 + 1 ];
        e21 = EE[2*9 + 1 ];
        e31 = EE[3*9 + 1 ];
        e02 = EE[0*9 + 2 ];
        e12 = EE[1*9 + 2 ];
        e22 = EE[2*9 + 2 ];
        e32 = EE[3*9 + 2 ];
        e03 = EE[0*9 + 3 ];
        e13 = EE[1*9 + 3 ];
        e23 = EE[2*9 + 3 ];
        e33 = EE[3*9 + 3 ];
        e04 = EE[0*9 + 4 ];
        e14 = EE[1*9 + 4 ];
        e24 = EE[2*9 + 4 ];
        e34 = EE[3*9 + 4 ];
        e05 = EE[0*9 + 5 ];
        e15 = EE[1*9 + 5 ];
        e25 = EE[2*9 + 5 ];
        e35 = EE[3*9 + 5 ];
        e06 = EE[0*9 + 6 ];
        e16 = EE[1*9 + 6 ];
        e26 = EE[2*9 + 6 ];
        e36 = EE[3*9 + 6 ];
        e07 = EE[0*9 + 7 ];
        e17 = EE[1*9 + 7 ];
        e27 = EE[2*9 + 7 ];
        e37 = EE[3*9 + 7 ];
        e08 = EE[0*9 + 8 ];
        e18 = EE[1*9 + 8 ];
        e28 = EE[2*9 + 8 ];
        e38 = EE[3*9 + 8 ];


        e002 =e00*e00;
        e102 =e10*e10;
        e202 =e20*e20;
        e302 =e30*e30;
        e012 =e01*e01;
        e112 =e11*e11;
        e212 =e21*e21;
        e312 =e31*e31;
        e022 =e02*e02;
        e122 =e12*e12;
        e222 =e22*e22;
        e322 =e32*e32;
        e032 =e03*e03;
        e132 =e13*e13;
        e232 =e23*e23;
        e332 =e33*e33;
        e042 =e04*e04;
        e142 =e14*e14;
        e242 =e24*e24;
        e342 =e34*e34;
        e052 =e05*e05;
        e152 =e15*e15;
        e252 =e25*e25;
        e352 =e35*e35;
        e062 =e06*e06;
        e162 =e16*e16;
        e262 =e26*e26;
        e362 =e36*e36;
        e072 =e07*e07;
        e172 =e17*e17;
        e272 =e27*e27;
        e372 =e37*e37;
        e082 =e08*e08;
        e182 =e18*e18;
        e282 =e28*e28;
        e382 =e38*e38;

        e003 =e00*e00*e00;
        e103 =e10*e10*e10;
        e203 =e20*e20*e20;
        e303 =e30*e30*e30;
        e013 =e01*e01*e01;
        e113 =e11*e11*e11;
        e213 =e21*e21*e21;
        e313 =e31*e31*e31;
        e023 =e02*e02*e02;
        e123 =e12*e12*e12;
        e223 =e22*e22*e22;
        e323 =e32*e32*e32;
        e033 =e03*e03*e03;
        e133 =e13*e13*e13;
        e233 =e23*e23*e23;
        e333 =e33*e33*e33;
        e043 =e04*e04*e04;
        e143 =e14*e14*e14;
        e243 =e24*e24*e24;
        e343 =e34*e34*e34;
        e053 =e05*e05*e05;
        e153 =e15*e15*e15;
        e253 =e25*e25*e25;
        e353 =e35*e35*e35;
        e063 =e06*e06*e06;
        e163 =e16*e16*e16;
        e263 =e26*e26*e26;
        e363 =e36*e36*e36;
        e073 =e07*e07*e07;
        e173 =e17*e17*e17;
        e273 =e27*e27*e27;
        e373 =e37*e37*e37;
        e083 =e08*e08*e08;
        e183 =e18*e18*e18;
        e283 =e28*e28*e28;
        e383 =e38*e38*e38;
    }

    // Build the vector A.
    {
        A[0 + 10*0]=0.5*e003+0.5*e00*e012+0.5*e00*e022+0.5*e00*e032+e03*e01*e04+e03*e02*e05+0.5*e00*e062+e06*e01*e07+e06*e02*e08-0.5*e00*e042-0.5*e00*e052-0.5*e00*e072-0.5*e00*e082;
        A[0 + 10*1]=e00*e11*e01+e00*e12*e02+e03*e00*e13+e03*e11*e04+e03*e01*e14+e03*e12*e05+e03*e02*e15+e13*e01*e04+e13*e02*e05+e06*e00*e16+1.5*e10*e002+0.5*e10*e012+0.5*e10*e022+0.5*e10*e062-0.5*e10*e042-0.5*e10*e052-0.5*e10*e072+0.5*e10*e032+e06*e11*e07+e06*e01*e17+e06*e12*e08+e06*e02*e18+e16*e01*e07+e16*e02*e08-e00*e14*e04-e00*e17*e07-e00*e15*e05-e00*e18*e08-0.5*e10*e082;
        A[0 + 10*2]=e16*e02*e18+e03*e12*e15+e10*e11*e01+e10*e12*e02+e03*e10*e13+e03*e11*e14+e13*e11*e04+e13*e01*e14+e13*e12*e05+e13*e02*e15+e06*e10*e16+e06*e12*e18+e06*e11*e17+e16*e11*e07+e16*e01*e17+e16*e12*e08-e10*e14*e04-e10*e17*e07-e10*e15*e05-e10*e18*e08+1.5*e00*e102+0.5*e00*e122+0.5*e00*e112+0.5*e00*e132+0.5*e00*e162-0.5*e00*e152-0.5*e00*e172-0.5*e00*e182-0.5*e00*e142;
        A[0 + 10*3]=0.5*e103+0.5*e10*e122+0.5*e10*e112+0.5*e10*e132+e13*e12*e15+e13*e11*e14+0.5*e10*e162+e16*e12*e18+e16*e11*e17-0.5*e10*e152-0.5*e10*e172-0.5*e10*e182-0.5*e10*e142;
        A[0 + 10*4]=-e00*e28*e08-e00*e25*e05-e00*e27*e07-e00*e24*e04+e26*e02*e08+e26*e01*e07+e06*e02*e28+e06*e22*e08+e06*e01*e27+e06*e21*e07+e23*e02*e05+e23*e01*e04+e03*e02*e25+e03*e22*e05+e03*e01*e24+e03*e21*e04+e00*e22*e02+e00*e21*e01-0.5*e20*e082-0.5*e20*e052-0.5*e20*e072-0.5*e20*e042+e06*e00*e26+0.5*e20*e062+e03*e00*e23+0.5*e20*e022+1.5*e20*e002+0.5*e20*e032+0.5*e20*e012;
        A[0 + 10*5]=-e10*e24*e04-e10*e27*e07-e10*e25*e05-e10*e28*e08-e20*e14*e04-e20*e17*e07-e20*e15*e05-e20*e18*e08-e00*e24*e14-e00*e25*e15-e00*e27*e17-e00*e28*e18+e06*e21*e17+e06*e22*e18+e06*e12*e28+e16*e00*e26+e16*e21*e07+e16*e01*e27+e16*e22*e08+e16*e02*e28+e26*e11*e07+e26*e01*e17+e26*e12*e08+e26*e02*e18+e06*e11*e27+e23*e11*e04+e23*e01*e14+e23*e12*e05+e23*e02*e15+e06*e20*e16+e06*e10*e26+e03*e21*e14+e03*e22*e15+e03*e12*e25+e13*e00*e23+e13*e21*e04+e13*e01*e24+e13*e22*e05+e13*e02*e25+e03*e11*e24+e03*e20*e13+e03*e10*e23+e00*e21*e11+3*e00*e20*e10+e00*e22*e12+e20*e12*e02+e20*e11*e01+e10*e22*e02+e10*e21*e01;
        A[0 + 10*6]=-0.5*e20*e152+e26*e11*e17-e10*e24*e14-e10*e25*e15-e10*e27*e17-e10*e28*e18+0.5*e20*e162+e13*e10*e23+e13*e22*e15+e23*e12*e15+e23*e11*e14+e16*e10*e26+e16*e21*e17+e16*e11*e27+e16*e22*e18+e16*e12*e28+e26*e12*e18+e13*e12*e25+0.5*e20*e132+1.5*e20*e102+0.5*e20*e122+0.5*e20*e112+e10*e21*e11+e10*e22*e12+e13*e11*e24-0.5*e20*e172-0.5*e20*e182-0.5*e20*e142+e13*e21*e14;
        A[0 + 10*7]=-e20*e25*e05-e20*e28*e08-0.5*e00*e272-0.5*e00*e282-0.5*e00*e242+0.5*e00*e262-0.5*e00*e252+e06*e20*e26+0.5*e00*e232+e06*e22*e28+e06*e21*e27+e26*e21*e07+e26*e01*e27+e26*e22*e08+e26*e02*e28-e20*e24*e04-e20*e27*e07+e03*e20*e23+e03*e22*e25+e03*e21*e24+e23*e21*e04+e23*e01*e24+e23*e22*e05+e23*e02*e25+e20*e21*e01+e20*e22*e02+1.5*e00*e202+0.5*e00*e222+0.5*e00*e212;
        A[0 + 10*8]=e23*e21*e14+e23*e11*e24+e23*e22*e15+e23*e12*e25+e16*e20*e26+e16*e22*e28+e16*e21*e27+e26*e21*e17+e26*e11*e27+e26*e22*e18+e26*e12*e28+1.5*e10*e202+0.5*e10*e222+0.5*e10*e212+0.5*e10*e232+e20*e21*e11+e20*e22*e12+e13*e20*e23+e13*e22*e25+e13*e21*e24-e20*e24*e14-e20*e25*e15-e20*e27*e17-e20*e28*e18-0.5*e10*e272-0.5*e10*e282-0.5*e10*e242-0.5*e10*e252+0.5*e10*e262;
        A[0 + 10*9]=0.5*e203+0.5*e20*e222+0.5*e20*e212+0.5*e20*e232+e23*e22*e25+e23*e21*e24+0.5*e20*e262+e26*e22*e28+e26*e21*e27-0.5*e20*e252-0.5*e20*e272-0.5*e20*e282-0.5*e20*e242;
        A[0 + 10*10]=e06*e32*e08-0.5*e30*e082-0.5*e30*e042-0.5*e30*e052-0.5*e30*e072+0.5*e30*e012+0.5*e30*e022+0.5*e30*e032+0.5*e30*e062+1.5*e30*e002+e00*e31*e01+e00*e32*e02+e03*e31*e04+e03*e01*e34+e03*e32*e05+e03*e02*e35+e33*e01*e04+e33*e02*e05+e06*e00*e36+e06*e31*e07+e06*e01*e37+e06*e02*e38+e36*e01*e07+e36*e02*e08-e00*e34*e04-e00*e37*e07-e00*e35*e05-e00*e38*e08+e03*e00*e33;
        A[0 + 10*11]=e06*e30*e16+e03*e30*e13+e16*e31*e07+e06*e10*e36-e10*e37*e07+3*e00*e30*e10+e00*e32*e12-e00*e38*e18-e10*e34*e04-e10*e35*e05-e10*e38*e08-e30*e14*e04-e30*e17*e07-e30*e15*e05-e30*e18*e08+e00*e31*e11+e10*e31*e01+e10*e32*e02+e30*e11*e01+e30*e12*e02+e03*e10*e33-e00*e34*e14-e00*e35*e15-e00*e37*e17+e03*e31*e14+e03*e11*e34+e03*e32*e15+e03*e12*e35+e13*e00*e33+e13*e31*e04+e13*e01*e34+e13*e32*e05+e13*e02*e35+e33*e11*e04+e33*e01*e14+e33*e12*e05+e33*e02*e15+e06*e31*e17+e06*e11*e37+e06*e32*e18+e06*e12*e38+e16*e00*e36+e16*e01*e37+e16*e32*e08+e16*e02*e38+e36*e11*e07+e36*e01*e17+e36*e12*e08+e36*e02*e18;
        A[0 + 10*12]=e13*e10*e33+e33*e11*e14+e16*e10*e36+e16*e31*e17+e16*e11*e37+e16*e32*e18+e16*e12*e38+e36*e12*e18+e36*e11*e17-e10*e34*e14-e10*e35*e15-e10*e37*e17-e10*e38*e18+e10*e31*e11+e10*e32*e12+e13*e31*e14+e13*e11*e34+e13*e32*e15+e13*e12*e35+e33*e12*e15+1.5*e30*e102+0.5*e30*e122+0.5*e30*e112+0.5*e30*e132+0.5*e30*e162-0.5*e30*e152-0.5*e30*e172-0.5*e30*e182-0.5*e30*e142;
        A[0 + 10*13]=e00*e32*e22+3*e00*e30*e20+e00*e31*e21+e20*e31*e01+e20*e32*e02+e30*e21*e01+e30*e22*e02+e03*e20*e33+e03*e32*e25+e03*e22*e35+e03*e31*e24+e03*e21*e34+e23*e00*e33+e23*e31*e04+e23*e01*e34+e23*e32*e05+e23*e02*e35+e33*e21*e04+e33*e01*e24+e33*e22*e05+e33*e02*e25+e06*e30*e26+e06*e20*e36+e06*e32*e28+e06*e22*e38+e06*e31*e27+e06*e21*e37+e26*e00*e36+e26*e31*e07+e03*e30*e23+e26*e01*e37+e26*e32*e08+e26*e02*e38+e36*e21*e07+e36*e01*e27+e36*e22*e08+e36*e02*e28-e00*e35*e25-e00*e37*e27-e00*e38*e28-e00*e34*e24-e20*e34*e04-e20*e37*e07-e20*e35*e05-e20*e38*e08-e30*e24*e04-e30*e27*e07-e30*e25*e05-e30*e28*e08;
        A[0 + 10*14]=e16*e30*e26+e13*e21*e34+3*e10*e30*e20+e10*e32*e22+e10*e31*e21+e20*e31*e11+e20*e32*e12+e30*e21*e11+e30*e22*e12+e13*e30*e23+e13*e20*e33+e13*e32*e25+e13*e22*e35+e13*e31*e24+e23*e10*e33+e23*e31*e14+e23*e11*e34+e23*e32*e15+e23*e12*e35+e33*e21*e14+e33*e11*e24+e33*e22*e15+e33*e12*e25+e16*e20*e36+e16*e32*e28+e16*e22*e38+e16*e31*e27+e16*e21*e37+e26*e10*e36+e26*e31*e17+e26*e11*e37+e26*e32*e18+e26*e12*e38+e36*e21*e17+e36*e11*e27+e36*e22*e18+e36*e12*e28-e10*e35*e25-e10*e37*e27-e10*e38*e28-e10*e34*e24-e20*e34*e14-e20*e35*e15-e20*e37*e17-e20*e38*e18-e30*e24*e14-e30*e25*e15-e30*e27*e17-e30*e28*e18;
        A[0 + 10*15]=-e20*e34*e24+0.5*e30*e262-0.5*e30*e252-0.5*e30*e272-0.5*e30*e282-0.5*e30*e242+1.5*e30*e202+0.5*e30*e222+0.5*e30*e212+0.5*e30*e232+e20*e32*e22+e20*e31*e21+e23*e20*e33+e23*e32*e25+e23*e22*e35+e23*e31*e24+e23*e21*e34+e33*e22*e25+e33*e21*e24+e26*e20*e36+e26*e32*e28+e26*e22*e38+e26*e31*e27+e26*e21*e37+e36*e22*e28+e36*e21*e27-e20*e35*e25-e20*e37*e27-e20*e38*e28;
        A[0 + 10*16]=0.5*e00*e322+e30*e32*e02+e30*e31*e01+1.5*e00*e302+0.5*e00*e312+e03*e32*e35+e33*e31*e04+e33*e01*e34+e33*e32*e05+e33*e02*e35+e06*e30*e36+e06*e31*e37+e06*e32*e38+e36*e31*e07+e36*e01*e37+e36*e32*e08+e36*e02*e38-e30*e34*e04-e30*e37*e07-e30*e35*e05-e30*e38*e08+0.5*e00*e332+0.5*e00*e362-0.5*e00*e382-0.5*e00*e352-0.5*e00*e342-0.5*e00*e372+e03*e30*e33+e03*e31*e34;
        A[0 + 10*17]=0.5*e10*e362-0.5*e10*e382-0.5*e10*e352-0.5*e10*e342-0.5*e10*e372+e36*e31*e17+e36*e11*e37+e36*e32*e18+e36*e12*e38-e30*e34*e14-e30*e35*e15-e30*e37*e17-e30*e38*e18+1.5*e10*e302+0.5*e10*e312+0.5*e10*e322+0.5*e10*e332+e30*e31*e11+e30*e32*e12+e13*e30*e33+e13*e31*e34+e13*e32*e35+e33*e31*e14+e33*e11*e34+e33*e32*e15+e33*e12*e35+e16*e30*e36+e16*e31*e37+e16*e32*e38;
        A[0 + 10*18]=e33*e31*e24+e33*e21*e34+e26*e30*e36+e26*e31*e37+e26*e32*e38+e36*e32*e28+e36*e22*e38+e36*e31*e27+e36*e21*e37-e30*e35*e25-e30*e37*e27-e30*e38*e28-e30*e34*e24+e33*e22*e35+1.5*e20*e302+0.5*e20*e312+0.5*e20*e322+0.5*e20*e332+0.5*e20*e362-0.5*e20*e382-0.5*e20*e352-0.5*e20*e342-0.5*e20*e372+e30*e32*e22+e30*e31*e21+e23*e30*e33+e23*e31*e34+e23*e32*e35+e33*e32*e25;
        A[0 + 10*19]=0.5*e303+0.5*e30*e312+0.5*e30*e322+0.5*e30*e332+e33*e31*e34+e33*e32*e35+0.5*e30*e362+e36*e31*e37+e36*e32*e38-0.5*e30*e382-0.5*e30*e352-0.5*e30*e342-0.5*e30*e372;
        A[1 + 10*0]=e00*e01*e04+0.5*e002*e03+e00*e02*e05+0.5*e033+0.5*e03*e042+0.5*e03*e052+0.5*e03*e062+e06*e04*e07+e06*e05*e08-0.5*e03*e012-0.5*e03*e072-0.5*e03*e022-0.5*e03*e082;
        A[1 + 10*1]=e03*e14*e04+e10*e01*e04+e16*e05*e08+e00*e10*e03+e00*e11*e04+e00*e01*e14+e00*e12*e05+e00*e02*e15+e10*e02*e05+e03*e15*e05+e06*e03*e16+e06*e14*e07+e06*e04*e17+e06*e15*e08+e06*e05*e18+0.5*e002*e13+1.5*e13*e032+0.5*e13*e042+0.5*e13*e052+0.5*e13*e062-0.5*e13*e012-0.5*e13*e072-0.5*e13*e022-0.5*e13*e082+e16*e04*e07-e03*e12*e02-e03*e11*e01-e03*e17*e07-e03*e18*e08;
        A[1 + 10*2]=-e13*e11*e01+e00*e10*e13+e00*e12*e15+e00*e11*e14+e10*e11*e04+e10*e01*e14+e10*e12*e05+e10*e02*e15+e13*e14*e04+e13*e15*e05+e06*e13*e16+e06*e15*e18+e06*e14*e17+e16*e14*e07+e16*e04*e17+e16*e15*e08+e16*e05*e18-e13*e12*e02-e13*e17*e07-e13*e18*e08+0.5*e102*e03+1.5*e03*e132+0.5*e03*e152+0.5*e03*e142+0.5*e03*e162-0.5*e03*e112-0.5*e03*e172-0.5*e03*e122-0.5*e03*e182;
        A[1 + 10*3]=0.5*e102*e13+e10*e11*e14+e10*e12*e15+0.5*e133+0.5*e13*e152+0.5*e13*e142+0.5*e13*e162+e16*e15*e18+e16*e14*e17-0.5*e13*e112-0.5*e13*e122-0.5*e13*e172-0.5*e13*e182;
        A[1 + 10*4]=-e03*e28*e08-e03*e27*e07-e03*e21*e01-e03*e22*e02+e26*e05*e08+e26*e04*e07+e06*e05*e28+e06*e25*e08+e06*e04*e27+e06*e24*e07+e03*e25*e05+e03*e24*e04+e20*e02*e05+e20*e01*e04+e00*e02*e25+e00*e22*e05+e00*e01*e24+e00*e21*e04+e00*e20*e03-0.5*e23*e072-0.5*e23*e082-0.5*e23*e022-0.5*e23*e012+e06*e03*e26+0.5*e23*e052+0.5*e23*e062+1.5*e23*e032+0.5*e23*e042+0.5*e002*e23;
        A[1 + 10*5]=e00*e21*e14+e00*e11*e24+e00*e10*e23+e00*e22*e15+e00*e12*e25+e20*e12*e05+e20*e01*e14+e20*e11*e04+e00*e20*e13+e10*e02*e25+e10*e22*e05+e10*e01*e24+e10*e21*e04+e10*e20*e03+e23*e15*e05+e23*e14*e04+e13*e25*e05+e13*e24*e04+e03*e24*e14+e03*e25*e15+3*e03*e23*e13+e20*e02*e15+e16*e03*e26+e06*e14*e27-e23*e18*e08+e06*e24*e17+e06*e15*e28+e06*e25*e18+e06*e13*e26+e06*e23*e16+e26*e04*e17+e26*e14*e07+e16*e05*e28+e16*e25*e08+e16*e04*e27+e16*e24*e07-e03*e22*e12-e03*e21*e11+e26*e05*e18+e26*e15*e08-e03*e27*e17-e03*e28*e18-e13*e22*e02-e13*e28*e08-e13*e27*e07-e13*e21*e01-e23*e17*e07-e23*e11*e01-e23*e12*e02;
        A[1 + 10*6]=-0.5*e23*e182-0.5*e23*e172-0.5*e23*e112-0.5*e23*e122-e13*e22*e12-e13*e27*e17-e13*e28*e18+e26*e15*e18+e26*e14*e17-e13*e21*e11+e20*e12*e15+e13*e25*e15+e13*e24*e14+e16*e13*e26+e16*e25*e18+e16*e15*e28+e16*e24*e17+e16*e14*e27+1.5*e23*e132+0.5*e23*e152+0.5*e23*e142+0.5*e23*e162+e10*e20*e13+e10*e21*e14+e10*e11*e24+e10*e22*e15+e10*e12*e25+e20*e11*e14+0.5*e102*e23;
        A[1 + 10*7]=e26*e04*e27+e00*e22*e25-e23*e28*e08+0.5*e03*e262-0.5*e03*e212-0.5*e03*e272-0.5*e03*e222-0.5*e03*e282+e23*e24*e04+e23*e25*e05+0.5*e202*e03+e06*e23*e26+e06*e24*e27+e06*e25*e28+e26*e24*e07+e26*e25*e08+e26*e05*e28-e23*e22*e02-e23*e21*e01-e23*e27*e07+e00*e20*e23+e00*e21*e24+e20*e21*e04+e20*e01*e24+e20*e22*e05+e20*e02*e25+1.5*e03*e232+0.5*e03*e242+0.5*e03*e252;
        A[1 + 10*8]=e20*e11*e24-0.5*e13*e212-0.5*e13*e272-0.5*e13*e222-0.5*e13*e282-e23*e27*e17-e23*e28*e18+e26*e25*e18+e26*e24*e17+e26*e14*e27-e23*e21*e11-e23*e22*e12+e26*e15*e28+e23*e25*e15+e23*e24*e14+e16*e23*e26+e16*e24*e27+e16*e25*e28+0.5*e13*e262+e20*e21*e14+e20*e22*e15+e20*e12*e25+0.5*e13*e242+0.5*e13*e252+0.5*e202*e13+1.5*e13*e232+e10*e20*e23+e10*e22*e25+e10*e21*e24;
        A[1 + 10*9]=0.5*e202*e23+e20*e22*e25+e20*e21*e24+0.5*e233+0.5*e23*e242+0.5*e23*e252+0.5*e23*e262+e26*e24*e27+e26*e25*e28-0.5*e23*e212-0.5*e23*e272-0.5*e23*e222-0.5*e23*e282;
        A[1 + 10*10]=e00*e30*e03+0.5*e33*e062-0.5*e33*e012-0.5*e33*e022-0.5*e33*e072+e03*e35*e05+e06*e03*e36+e06*e34*e07+e06*e04*e37+e06*e35*e08+e06*e05*e38+e36*e04*e07+e36*e05*e08-e03*e32*e02-e03*e31*e01-e03*e37*e07+e00*e31*e04+e00*e01*e34+e00*e32*e05+e00*e02*e35+e30*e01*e04+e30*e02*e05+e03*e34*e04-e03*e38*e08+0.5*e002*e33+1.5*e33*e032+0.5*e33*e042+0.5*e33*e052-0.5*e33*e082;
        A[1 + 10*11]=e06*e35*e18+e06*e33*e16+e00*e30*e13+e00*e10*e33+e00*e31*e14+e00*e11*e34+e00*e32*e15+e00*e12*e35+e10*e30*e03-e33*e17*e07-e33*e18*e08+e10*e31*e04+e10*e01*e34+e10*e32*e05+e10*e02*e35+e30*e11*e04+e30*e01*e14+e30*e12*e05+e30*e02*e15+3*e03*e33*e13+e03*e35*e15+e03*e34*e14+e13*e34*e04+e13*e35*e05+e33*e14*e04+e33*e15*e05+e06*e13*e36+e06*e15*e38+e06*e34*e17+e06*e14*e37+e16*e03*e36+e16*e34*e07+e16*e04*e37+e16*e35*e08+e16*e05*e38+e36*e14*e07+e36*e04*e17+e36*e15*e08+e36*e05*e18-e03*e31*e11-e03*e32*e12-e03*e37*e17-e03*e38*e18-e13*e32*e02-e13*e31*e01-e13*e37*e07-e13*e38*e08-e33*e12*e02-e33*e11*e01;
        A[1 + 10*12]=e16*e13*e36+e10*e11*e34+0.5*e33*e152+0.5*e33*e142+0.5*e33*e162-0.5*e33*e112-0.5*e33*e122-0.5*e33*e172-0.5*e33*e182+0.5*e102*e33+1.5*e33*e132+e10*e30*e13+e10*e31*e14+e10*e32*e15+e10*e12*e35+e30*e11*e14+e30*e12*e15+e13*e35*e15+e13*e34*e14+e16*e35*e18+e16*e15*e38+e16*e34*e17+e16*e14*e37+e36*e15*e18+e36*e14*e17-e13*e31*e11-e13*e32*e12-e13*e37*e17-e13*e38*e18;
        A[1 + 10*13]=e06*e35*e28+e36*e04*e27+e00*e20*e33+e00*e30*e23+3*e03*e33*e23+e03*e34*e24+e03*e35*e25+e23*e34*e04+e23*e35*e05+e33*e24*e04+e33*e25*e05+e06*e33*e26+e06*e23*e36+e06*e34*e27+e06*e24*e37+e06*e25*e38+e26*e03*e36+e26*e34*e07+e26*e04*e37+e26*e35*e08+e26*e05*e38+e36*e24*e07+e36*e25*e08+e36*e05*e28-e03*e31*e21-e03*e37*e27-e03*e32*e22-e03*e38*e28-e23*e32*e02-e23*e31*e01-e23*e37*e07-e23*e38*e08-e33*e22*e02-e33*e21*e01-e33*e27*e07-e33*e28*e08+e00*e32*e25+e00*e22*e35+e00*e31*e24+e00*e21*e34+e20*e30*e03+e20*e31*e04+e20*e01*e34+e20*e32*e05+e20*e02*e35+e30*e21*e04+e30*e01*e24+e30*e22*e05+e30*e02*e25;
        A[1 + 10*14]=e10*e30*e23+e10*e20*e33+e10*e22*e35+e10*e32*e25+e10*e31*e24+e10*e21*e34+e20*e30*e13+e20*e31*e14+e20*e11*e34+e20*e32*e15+e20*e12*e35+e30*e21*e14+e30*e11*e24+e30*e22*e15+e30*e12*e25+3*e13*e33*e23+e13*e34*e24+e13*e35*e25+e23*e35*e15+e23*e34*e14+e33*e25*e15+e33*e24*e14+e16*e33*e26+e16*e23*e36+e16*e34*e27+e16*e24*e37+e16*e35*e28+e16*e25*e38+e26*e13*e36+e26*e35*e18+e26*e15*e38+e26*e34*e17+e26*e14*e37+e36*e25*e18+e36*e15*e28+e36*e24*e17+e36*e14*e27-e13*e31*e21-e13*e37*e27-e13*e32*e22-e13*e38*e28-e23*e31*e11-e23*e32*e12-e23*e37*e17-e23*e38*e18-e33*e21*e11-e33*e22*e12-e33*e27*e17-e33*e28*e18;
        A[1 + 10*15]=-0.5*e33*e212-0.5*e33*e272-0.5*e33*e222-0.5*e33*e282+e26*e23*e36+e20*e30*e23+e20*e32*e25+e20*e22*e35+e20*e31*e24+e20*e21*e34+e30*e22*e25+e30*e21*e24+e23*e34*e24+e23*e35*e25+e26*e34*e27+e26*e24*e37+e26*e35*e28+e26*e25*e38+e36*e24*e27+e36*e25*e28-e23*e31*e21-e23*e37*e27-e23*e32*e22-e23*e38*e28+0.5*e202*e33+1.5*e33*e232+0.5*e33*e242+0.5*e33*e252+0.5*e33*e262;
        A[1 + 10*16]=e33*e35*e05+e30*e32*e05+0.5*e03*e362+0.5*e302*e03+1.5*e03*e332+0.5*e03*e352+0.5*e03*e342+e00*e30*e33+e00*e31*e34+e00*e32*e35+e30*e31*e04+e30*e01*e34+e30*e02*e35+e33*e34*e04+e06*e33*e36+e06*e35*e38+e06*e34*e37+e36*e34*e07+e36*e04*e37+e36*e35*e08+e36*e05*e38-e33*e32*e02-e33*e31*e01-e33*e37*e07-e33*e38*e08-0.5*e03*e322-0.5*e03*e382-0.5*e03*e312-0.5*e03*e372;
        A[1 + 10*17]=-e33*e31*e11-e33*e32*e12-e33*e38*e18+e30*e11*e34+e30*e32*e15+e30*e12*e35+e33*e35*e15+e33*e34*e14+e16*e33*e36+e16*e35*e38+e16*e34*e37+e36*e35*e18+e36*e15*e38+e36*e34*e17+e36*e14*e37-e33*e37*e17+0.5*e302*e13+1.5*e13*e332+0.5*e13*e352+0.5*e13*e342+0.5*e13*e362-0.5*e13*e322-0.5*e13*e382-0.5*e13*e312-0.5*e13*e372+e10*e30*e33+e10*e31*e34+e10*e32*e35+e30*e31*e14;
        A[1 + 10*18]=e36*e25*e38+0.5*e302*e23+1.5*e23*e332+0.5*e23*e352+0.5*e23*e342+0.5*e23*e362-0.5*e23*e322-0.5*e23*e382-0.5*e23*e312-0.5*e23*e372+e20*e30*e33+e20*e31*e34+e20*e32*e35+e30*e32*e25+e30*e22*e35+e30*e31*e24+e30*e21*e34+e33*e34*e24+e33*e35*e25+e26*e33*e36+e26*e35*e38+e26*e34*e37+e36*e34*e27+e36*e24*e37+e36*e35*e28-e33*e31*e21-e33*e37*e27-e33*e32*e22-e33*e38*e28;
        A[1 + 10*19]=0.5*e302*e33+e30*e31*e34+e30*e32*e35+0.5*e333+0.5*e33*e352+0.5*e33*e342+0.5*e33*e362+e36*e35*e38+e36*e34*e37-0.5*e33*e322-0.5*e33*e382-0.5*e33*e312-0.5*e33*e372;
        A[2 + 10*0]=0.5*e002*e06+e00*e01*e07+e00*e02*e08+0.5*e032*e06+e03*e04*e07+e03*e05*e08+0.5*e063+0.5*e06*e072+0.5*e06*e082-0.5*e06*e012-0.5*e06*e022-0.5*e06*e042-0.5*e06*e052;
        A[2 + 10*1]=e00*e10*e06+0.5*e002*e16+0.5*e032*e16+1.5*e16*e062+0.5*e16*e072+0.5*e16*e082-0.5*e16*e012-0.5*e16*e022-0.5*e16*e042-0.5*e16*e052+e00*e11*e07+e00*e01*e17+e00*e12*e08+e00*e02*e18+e10*e01*e07+e10*e02*e08+e03*e13*e06+e03*e14*e07+e03*e04*e17+e03*e15*e08+e03*e05*e18+e13*e04*e07+e13*e05*e08+e06*e17*e07+e06*e18*e08-e06*e12*e02-e06*e11*e01-e06*e14*e04-e06*e15*e05;
        A[2 + 10*2]=e13*e14*e07+0.5*e102*e06+e00*e10*e16+e00*e12*e18+e00*e11*e17+e10*e11*e07+e10*e01*e17+e10*e12*e08+e10*e02*e18+e03*e13*e16+e03*e15*e18+e03*e14*e17+e13*e04*e17+e13*e15*e08+e13*e05*e18+e16*e17*e07+e16*e18*e08-e16*e12*e02-e16*e11*e01-e16*e14*e04-e16*e15*e05+0.5*e132*e06+1.5*e06*e162+0.5*e06*e182+0.5*e06*e172-0.5*e06*e112-0.5*e06*e122-0.5*e06*e142-0.5*e06*e152;
        A[2 + 10*3]=0.5*e102*e16+e10*e12*e18+e10*e11*e17+0.5*e132*e16+e13*e15*e18+e13*e14*e17+0.5*e163+0.5*e16*e182+0.5*e16*e172-0.5*e16*e112-0.5*e16*e122-0.5*e16*e142-0.5*e16*e152;
        A[2 + 10*4]=e06*e27*e07+e23*e05*e08+e23*e04*e07+e03*e05*e28+e03*e25*e08+e03*e04*e27+e03*e24*e07+e20*e02*e08+e20*e01*e07+e00*e02*e28+e00*e22*e08+e00*e01*e27+e00*e21*e07+e00*e20*e06-e06*e25*e05-e06*e24*e04-e06*e21*e01-e06*e22*e02+e06*e28*e08-0.5*e26*e042-0.5*e26*e052-0.5*e26*e012-0.5*e26*e022+0.5*e26*e082+0.5*e26*e072+1.5*e26*e062+0.5*e002*e26+e03*e23*e06+0.5*e032*e26;
        A[2 + 10*5]=e13*e05*e28+e00*e12*e28+e13*e25*e08+e13*e04*e27+e13*e24*e07+e13*e23*e06+e03*e14*e27+e03*e24*e17+e03*e15*e28+e03*e25*e18+e03*e13*e26+e03*e23*e16+e20*e02*e18+e20*e12*e08+e20*e01*e17+e20*e11*e07+e00*e21*e17+e10*e02*e28+e10*e22*e08+e10*e01*e27+e10*e21*e07+e10*e20*e06+e00*e11*e27-e26*e15*e05-e26*e14*e04-e26*e11*e01-e26*e12*e02-e16*e25*e05-e16*e24*e04-e16*e21*e01-e16*e22*e02-e06*e24*e14-e06*e22*e12-e06*e21*e11-e06*e25*e15+e00*e20*e16+e00*e22*e18+e00*e10*e26+e26*e18*e08+e26*e17*e07+e16*e28*e08+e16*e27*e07+e06*e27*e17+e06*e28*e18+3*e06*e26*e16+e23*e05*e18+e23*e15*e08+e23*e04*e17+e23*e14*e07;
        A[2 + 10*6]=e10*e22*e18+0.5*e26*e182+0.5*e26*e172+e16*e28*e18+e16*e27*e17-e16*e25*e15-e16*e21*e11-e16*e22*e12+1.5*e26*e162+e13*e15*e28+e13*e24*e17+e13*e14*e27+e23*e15*e18+e23*e14*e17+e10*e12*e28+e10*e21*e17+e10*e11*e27+e20*e12*e18+e20*e11*e17+e13*e23*e16+e13*e25*e18+e10*e20*e16+0.5*e102*e26-0.5*e26*e122-0.5*e26*e142-0.5*e26*e152-e16*e24*e14-0.5*e26*e112+0.5*e132*e26;
        A[2 + 10*7]=-0.5*e06*e212-0.5*e06*e252-0.5*e06*e242+0.5*e06*e272+0.5*e06*e282-0.5*e06*e222+e20*e02*e28+e03*e23*e26+e03*e24*e27+e03*e25*e28+e23*e24*e07+e23*e04*e27+e23*e25*e08+e23*e05*e28+e26*e28*e08-e26*e22*e02-e26*e21*e01-e26*e24*e04-e26*e25*e05+e26*e27*e07+e00*e20*e26+e00*e21*e27+e00*e22*e28+e20*e21*e07+e20*e01*e27+e20*e22*e08+0.5*e202*e06+0.5*e232*e06+1.5*e06*e262;
        A[2 + 10*8]=-e26*e24*e14-0.5*e16*e212-0.5*e16*e252-0.5*e16*e242-e26*e25*e15-0.5*e16*e222-e26*e21*e11+e26*e28*e18+e26*e27*e17-e26*e22*e12+e23*e15*e28+e23*e24*e17+e23*e14*e27+0.5*e232*e16+1.5*e16*e262+0.5*e16*e272+0.5*e16*e282+e10*e20*e26+e10*e21*e27+e10*e22*e28+e20*e22*e18+e20*e12*e28+e20*e21*e17+e20*e11*e27+e13*e23*e26+e13*e24*e27+e13*e25*e28+e23*e25*e18+0.5*e202*e16;
        A[2 + 10*9]=0.5*e202*e26+e20*e21*e27+e20*e22*e28+0.5*e232*e26+e23*e24*e27+e23*e25*e28+0.5*e263+0.5*e26*e272+0.5*e26*e282-0.5*e26*e222-0.5*e26*e212-0.5*e26*e252-0.5*e26*e242;
        A[2 + 10*10]=e03*e34*e07+0.5*e032*e36+1.5*e36*e062+e03*e33*e06+e00*e31*e07+e00*e01*e37+e00*e32*e08+e00*e02*e38+e30*e01*e07+e30*e02*e08+e03*e04*e37+e03*e35*e08+e03*e05*e38+0.5*e002*e36-0.5*e36*e022-0.5*e36*e042-0.5*e36*e052+0.5*e36*e072+0.5*e36*e082-0.5*e36*e012+e33*e04*e07+e33*e05*e08+e06*e37*e07+e06*e38*e08-e06*e32*e02-e06*e31*e01-e06*e34*e04-e06*e35*e05+e00*e30*e06;
        A[2 + 10*11]=e13*e33*e06+e13*e34*e07+e13*e04*e37+e13*e35*e08+e13*e05*e38+e33*e14*e07+e33*e04*e17+e33*e15*e08+e33*e05*e18+3*e06*e36*e16+e06*e38*e18+e06*e37*e17+e16*e37*e07+e16*e38*e08+e36*e17*e07+e36*e18*e08-e06*e35*e15-e06*e31*e11-e06*e32*e12+e00*e31*e17+e00*e11*e37+e10*e30*e06+e10*e31*e07+e10*e01*e37+e10*e32*e08+e10*e02*e38+e30*e11*e07+e30*e01*e17+e30*e12*e08+e30*e02*e18+e03*e33*e16+e03*e13*e36+e03*e35*e18+e03*e15*e38+e03*e34*e17+e03*e14*e37+e00*e30*e16+e00*e12*e38-e06*e34*e14-e16*e32*e02-e16*e31*e01-e16*e34*e04-e16*e35*e05-e36*e12*e02-e36*e11*e01-e36*e14*e04-e36*e15*e05+e00*e10*e36+e00*e32*e18;
        A[2 + 10*12]=0.5*e36*e182+0.5*e36*e172-0.5*e36*e112-0.5*e36*e122-0.5*e36*e142-0.5*e36*e152+0.5*e102*e36+0.5*e132*e36+1.5*e36*e162+e10*e30*e16+e10*e32*e18+e10*e12*e38+e10*e31*e17+e10*e11*e37+e30*e12*e18+e30*e11*e17+e13*e33*e16+e13*e35*e18+e13*e15*e38+e13*e34*e17+e13*e14*e37+e33*e15*e18+e33*e14*e17+e16*e38*e18+e16*e37*e17-e16*e35*e15-e16*e31*e11-e16*e32*e12-e16*e34*e14;
        A[2 + 10*13]=e00*e20*e36+e00*e31*e27+e00*e21*e37+e00*e32*e28+e00*e22*e38+e20*e30*e06+e20*e31*e07+e20*e01*e37+e20*e32*e08+e20*e02*e38+e30*e21*e07+e30*e01*e27+e30*e22*e08+e30*e02*e28+e03*e33*e26+e03*e23*e36+e03*e34*e27+e03*e24*e37+e03*e35*e28-e26*e31*e01-e26*e35*e05-e36*e22*e02-e36*e21*e01-e36*e24*e04-e36*e25*e05-e26*e34*e04+e03*e25*e38+e23*e34*e07+e23*e04*e37+e23*e35*e08+e23*e05*e38+e33*e24*e07+e33*e04*e27+e33*e25*e08+e33*e05*e28+3*e06*e36*e26+e06*e37*e27+e06*e38*e28+e26*e37*e07+e26*e38*e08+e36*e27*e07+e36*e28*e08-e06*e32*e22-e06*e31*e21-e06*e35*e25-e06*e34*e24-e26*e32*e02+e00*e30*e26+e23*e33*e06;
        A[2 + 10*14]=e10*e30*e26+e10*e20*e36+e10*e31*e27+e10*e21*e37+e10*e32*e28+e10*e22*e38+e20*e30*e16+e20*e32*e18+e20*e12*e38+e20*e31*e17+e20*e11*e37+e30*e22*e18+e30*e12*e28+e30*e21*e17+e30*e11*e27+e13*e33*e26+e13*e23*e36+e13*e34*e27+e13*e24*e37+e13*e35*e28+e13*e25*e38+e23*e33*e16+e23*e35*e18+e23*e15*e38+e23*e34*e17+e23*e14*e37+e33*e25*e18+e33*e15*e28+e33*e24*e17+e33*e14*e27+3*e16*e36*e26+e16*e37*e27+e16*e38*e28+e26*e38*e18+e26*e37*e17+e36*e28*e18+e36*e27*e17-e16*e32*e22-e16*e31*e21-e16*e35*e25-e16*e34*e24-e26*e35*e15-e26*e31*e11-e26*e32*e12-e26*e34*e14-e36*e25*e15-e36*e21*e11-e36*e22*e12-e36*e24*e14;
        A[2 + 10*15]=e33*e25*e28+e20*e30*e26+e20*e32*e28+e20*e31*e27+e20*e21*e37+e20*e22*e38+e30*e21*e27+e30*e22*e28+e23*e33*e26+e23*e34*e27+e23*e24*e37+e23*e35*e28+e23*e25*e38+e33*e24*e27+e26*e37*e27+e26*e38*e28-e26*e32*e22-e26*e31*e21-e26*e35*e25-e26*e34*e24+0.5*e202*e36+0.5*e232*e36+1.5*e36*e262+0.5*e36*e272+0.5*e36*e282-0.5*e36*e222-0.5*e36*e212-0.5*e36*e252-0.5*e36*e242;
        A[2 + 10*16]=e00*e30*e36+e00*e32*e38+e00*e31*e37+e30*e31*e07+e30*e01*e37+e30*e32*e08+e30*e02*e38+e03*e33*e36-0.5*e06*e342+e03*e35*e38+e33*e34*e07+e33*e04*e37+e33*e35*e08+e33*e05*e38+e36*e37*e07+e36*e38*e08-e36*e32*e02-e36*e31*e01-e36*e34*e04-e36*e35*e05+e03*e34*e37+0.5*e302*e06+0.5*e332*e06+1.5*e06*e362+0.5*e06*e382+0.5*e06*e372-0.5*e06*e352-0.5*e06*e312-0.5*e06*e322;
        A[2 + 10*17]=-e36*e35*e15+e10*e30*e36+0.5*e302*e16+0.5*e332*e16+1.5*e16*e362+0.5*e16*e382+0.5*e16*e372-0.5*e16*e352-0.5*e16*e312-0.5*e16*e322-0.5*e16*e342+e10*e32*e38+e10*e31*e37+e30*e32*e18+e30*e12*e38+e30*e31*e17+e30*e11*e37+e13*e33*e36+e13*e35*e38+e13*e34*e37+e33*e35*e18+e33*e15*e38+e33*e34*e17+e33*e14*e37+e36*e38*e18+e36*e37*e17-e36*e31*e11-e36*e32*e12-e36*e34*e14;
        A[2 + 10*18]=-e36*e35*e25+e30*e32*e28+0.5*e302*e26+0.5*e332*e26+1.5*e26*e362+0.5*e26*e382+0.5*e26*e372-0.5*e26*e352-0.5*e26*e312-0.5*e26*e322-0.5*e26*e342+e20*e30*e36+e20*e32*e38+e20*e31*e37+e30*e31*e27+e30*e21*e37+e30*e22*e38+e23*e33*e36+e23*e35*e38+e23*e34*e37+e33*e34*e27+e33*e24*e37+e33*e35*e28+e33*e25*e38+e36*e37*e27+e36*e38*e28-e36*e32*e22-e36*e31*e21-e36*e34*e24;
        A[2 + 10*19]=0.5*e302*e36+e30*e32*e38+e30*e31*e37+0.5*e332*e36+e33*e35*e38+e33*e34*e37+0.5*e363+0.5*e36*e382+0.5*e36*e372-0.5*e36*e352-0.5*e36*e312-0.5*e36*e322-0.5*e36*e342;
        A[3 + 10*0]=0.5*e01*e002+0.5*e013+0.5*e01*e022+e04*e00*e03+0.5*e01*e042+e04*e02*e05+e07*e00*e06+0.5*e01*e072+e07*e02*e08-0.5*e01*e032-0.5*e01*e052-0.5*e01*e062-0.5*e01*e082;
        A[3 + 10*1]=1.5*e11*e012+0.5*e11*e002+0.5*e11*e022+0.5*e11*e042+0.5*e11*e072-0.5*e11*e032-0.5*e11*e052-0.5*e11*e062-0.5*e11*e082+e01*e10*e00+e01*e12*e02+e04*e10*e03+e04*e00*e13+e04*e01*e14+e04*e12*e05+e04*e02*e15+e14*e00*e03+e14*e02*e05+e07*e10*e06+e07*e00*e16+e07*e01*e17+e07*e12*e08+e07*e02*e18+e17*e00*e06+e17*e02*e08-e01*e13*e03-e01*e16*e06-e01*e15*e05-e01*e18*e08;
        A[3 + 10*2]=e17*e02*e18+e14*e10*e03+e11*e12*e02-e11*e18*e08+0.5*e01*e102+0.5*e01*e122+1.5*e01*e112+0.5*e01*e142+0.5*e01*e172-0.5*e01*e132-0.5*e01*e152-0.5*e01*e162-0.5*e01*e182+e11*e10*e00+e04*e10*e13+e04*e12*e15+e04*e11*e14+e14*e00*e13+e14*e12*e05+e14*e02*e15+e07*e10*e16+e07*e12*e18+e07*e11*e17+e17*e10*e06+e17*e00*e16+e17*e12*e08-e11*e13*e03-e11*e16*e06-e11*e15*e05;
        A[3 + 10*3]=0.5*e11*e102+0.5*e11*e122+0.5*e113+e14*e10*e13+e14*e12*e15+0.5*e11*e142+e17*e10*e16+e17*e12*e18+0.5*e11*e172-0.5*e11*e132-0.5*e11*e152-0.5*e11*e162-0.5*e11*e182;
        A[3 + 10*4]=-e01*e25*e05-e01*e26*e06-e01*e23*e03+e27*e02*e08+e27*e00*e06+e07*e02*e28+e07*e22*e08+e07*e01*e27+e07*e00*e26+e24*e02*e05+e24*e00*e03+e04*e02*e25+e04*e22*e05+e04*e01*e24+e04*e00*e23+e04*e20*e03+e01*e22*e02+e01*e20*e00-e01*e28*e08+e07*e20*e06+0.5*e21*e072+0.5*e21*e042+0.5*e21*e022+0.5*e21*e002+1.5*e21*e012-0.5*e21*e082-0.5*e21*e052-0.5*e21*e062-0.5*e21*e032;
        A[3 + 10*5]=e11*e20*e00+e07*e20*e16+3*e01*e21*e11+e01*e22*e12-e21*e18*e08-e21*e15*e05-e21*e16*e06-e21*e13*e03-e11*e28*e08-e11*e25*e05-e11*e26*e06-e11*e23*e03-e01*e28*e18-e01*e23*e13-e01*e25*e15-e01*e26*e16+e27*e02*e18+e27*e12*e08+e27*e00*e16+e27*e10*e06+e17*e02*e28+e17*e22*e08+e17*e01*e27+e17*e00*e26+e17*e20*e06+e07*e11*e27+e07*e21*e17+e07*e12*e28+e07*e22*e18+e07*e10*e26+e24*e02*e15+e24*e12*e05+e24*e00*e13+e24*e10*e03+e14*e02*e25+e14*e22*e05+e14*e01*e24+e14*e00*e23+e14*e20*e03+e04*e11*e24+e04*e21*e14+e04*e12*e25+e04*e22*e15+e21*e12*e02+e04*e20*e13+e01*e20*e10+e11*e22*e02+e21*e10*e00+e04*e10*e23;
        A[3 + 10*6]=1.5*e21*e112+0.5*e21*e102+0.5*e21*e122+e11*e20*e10+e11*e22*e12+e14*e10*e23+e14*e22*e15+e14*e12*e25-0.5*e21*e162-0.5*e21*e152-0.5*e21*e132-0.5*e21*e182+e27*e12*e18-e11*e26*e16-e11*e25*e15-e11*e23*e13-e11*e28*e18+e17*e20*e16+e17*e10*e26+e17*e22*e18+e17*e12*e28+e17*e11*e27+e27*e10*e16+0.5*e21*e172+e14*e11*e24+e24*e10*e13+e24*e12*e15+0.5*e21*e142+e14*e20*e13;
        A[3 + 10*7]=-0.5*e01*e262-0.5*e01*e282-0.5*e01*e252-0.5*e01*e232+0.5*e01*e272+e27*e22*e08+e27*e02*e28-e21*e23*e03-e21*e26*e06-e21*e25*e05-e21*e28*e08+e04*e22*e25+e24*e20*e03+e24*e00*e23+e24*e22*e05+e24*e02*e25+e07*e20*e26+e07*e21*e27+e07*e22*e28+e27*e20*e06+e27*e00*e26+e21*e20*e00+e21*e22*e02+e04*e20*e23+e04*e21*e24+0.5*e01*e222+0.5*e01*e242+1.5*e01*e212+0.5*e01*e202;
        A[3 + 10*8]=-0.5*e11*e282-0.5*e11*e252-e21*e26*e16+e27*e12*e28-e21*e25*e15-e21*e23*e13-e21*e28*e18+e17*e20*e26+e17*e21*e27+e17*e22*e28+e27*e20*e16+e27*e10*e26+e27*e22*e18+0.5*e11*e242+0.5*e11*e272-0.5*e11*e232-0.5*e11*e262+0.5*e11*e202+1.5*e11*e212+0.5*e11*e222+e21*e20*e10+e14*e20*e23+e14*e21*e24+e14*e22*e25+e24*e20*e13+e24*e10*e23+e24*e22*e15+e24*e12*e25+e21*e22*e12;
        A[3 + 10*9]=0.5*e21*e202+0.5*e213+0.5*e21*e222+e24*e20*e23+0.5*e21*e242+e24*e22*e25+e27*e20*e26+0.5*e21*e272+e27*e22*e28-0.5*e21*e232-0.5*e21*e262-0.5*e21*e282-0.5*e21*e252;
        A[3 + 10*10]=-0.5*e31*e032-0.5*e31*e052-0.5*e31*e062-0.5*e31*e082+e07*e30*e06+e07*e00*e36+e07*e01*e37+e07*e32*e08+e07*e02*e38+e37*e00*e06+e37*e02*e08-e01*e33*e03-e01*e36*e06-e01*e35*e05-e01*e38*e08+0.5*e31*e072+e04*e30*e03+e04*e00*e33+e04*e01*e34+e04*e32*e05+e04*e02*e35+e34*e00*e03+e34*e02*e05+0.5*e31*e002+0.5*e31*e022+0.5*e31*e042+e01*e30*e00+e01*e32*e02+1.5*e31*e012;
        A[3 + 10*11]=e34*e12*e05+e34*e02*e15+e07*e10*e36+e07*e32*e18+e07*e12*e38+e07*e31*e17+e07*e11*e37+e17*e30*e06+e17*e00*e36+e17*e01*e37+e17*e32*e08+e17*e02*e38+e37*e10*e06+e37*e00*e16+e37*e12*e08+e37*e02*e18-e01*e36*e16-e01*e35*e15-e01*e33*e13-e01*e38*e18-e11*e33*e03-e11*e36*e06-e11*e35*e05+e01*e30*e10+e01*e32*e12+3*e01*e31*e11+e11*e30*e00+e11*e32*e02+e31*e10*e00+e31*e12*e02+e04*e30*e13+e04*e10*e33+e04*e32*e15+e04*e12*e35+e04*e31*e14+e04*e11*e34+e14*e30*e03+e14*e00*e33+e14*e01*e34+e14*e32*e05+e14*e02*e35+e34*e10*e03+e34*e00*e13+e07*e30*e16-e11*e38*e08-e31*e13*e03-e31*e16*e06-e31*e15*e05-e31*e18*e08;
        A[3 + 10*12]=-e11*e33*e13-e11*e38*e18+0.5*e31*e142+0.5*e31*e172-0.5*e31*e162-0.5*e31*e152-0.5*e31*e132-0.5*e31*e182+0.5*e31*e122+0.5*e31*e102+e11*e30*e10+e11*e32*e12+e14*e30*e13+e14*e10*e33+e14*e32*e15+e14*e12*e35+e14*e11*e34+e34*e10*e13+e34*e12*e15+e17*e30*e16+e17*e10*e36+e17*e32*e18+e17*e12*e38+e17*e11*e37+e37*e10*e16+e37*e12*e18-e11*e36*e16-e11*e35*e15+1.5*e31*e112;
        A[3 + 10*13]=-e21*e35*e05+e07*e32*e28+e01*e30*e20-e21*e33*e03-e21*e36*e06-e21*e38*e08-e31*e23*e03-e31*e26*e06-e31*e25*e05-e31*e28*e08+3*e01*e31*e21+e01*e32*e22+e21*e30*e00+e21*e32*e02+e31*e20*e00+e31*e22*e02+e04*e30*e23+e04*e20*e33+e04*e31*e24+e04*e21*e34+e04*e32*e25+e04*e22*e35+e24*e30*e03+e24*e00*e33+e24*e01*e34+e24*e32*e05+e24*e02*e35+e34*e20*e03+e34*e00*e23+e34*e22*e05+e34*e02*e25+e07*e30*e26+e07*e20*e36+e07*e31*e27+e07*e21*e37+e07*e22*e38+e27*e30*e06+e27*e00*e36+e27*e01*e37+e27*e32*e08+e27*e02*e38+e37*e00*e26+e37*e22*e08+e37*e02*e28-e01*e33*e23-e01*e36*e26-e01*e38*e28-e01*e35*e25+e37*e20*e06;
        A[3 + 10*14]=e11*e32*e22+e34*e12*e25+e11*e30*e20+3*e11*e31*e21+e21*e30*e10+e21*e32*e12+e34*e10*e23+e34*e22*e15+e17*e30*e26+e17*e20*e36+e17*e31*e27+e17*e21*e37+e17*e32*e28+e17*e22*e38+e27*e30*e16+e27*e10*e36+e27*e32*e18+e27*e12*e38+e27*e11*e37+e37*e20*e16+e37*e10*e26+e37*e22*e18+e37*e12*e28-e11*e33*e23-e11*e36*e26-e11*e38*e28-e11*e35*e25-e21*e36*e16-e21*e35*e15-e21*e33*e13-e21*e38*e18-e31*e26*e16-e31*e25*e15-e31*e23*e13-e31*e28*e18+e31*e20*e10+e31*e22*e12+e14*e30*e23+e14*e20*e33+e14*e31*e24+e14*e21*e34+e14*e32*e25+e14*e22*e35+e24*e30*e13+e24*e10*e33+e24*e32*e15+e24*e12*e35+e24*e11*e34+e34*e20*e13;
        A[3 + 10*15]=-e21*e36*e26+e37*e22*e28-e21*e33*e23-e21*e38*e28-e21*e35*e25+0.5*e31*e222+0.5*e31*e242+0.5*e31*e272-0.5*e31*e232-0.5*e31*e262-0.5*e31*e282-0.5*e31*e252+e21*e30*e20+e21*e32*e22+e24*e30*e23+e24*e20*e33+e24*e21*e34+e24*e32*e25+e24*e22*e35+e34*e20*e23+e34*e22*e25+e27*e30*e26+e27*e20*e36+e27*e21*e37+e27*e32*e28+e27*e22*e38+e37*e20*e26+1.5*e31*e212+0.5*e31*e202;
        A[3 + 10*16]=e04*e32*e35+0.5*e01*e372-0.5*e01*e352-0.5*e01*e362-0.5*e01*e332-0.5*e01*e382+e04*e31*e34+e34*e30*e03+e34*e00*e33+e34*e32*e05+e34*e02*e35+e07*e30*e36+e07*e32*e38+e07*e31*e37+e37*e30*e06+e37*e00*e36+e37*e32*e08+e04*e30*e33+e37*e02*e38-e31*e33*e03-e31*e36*e06-e31*e35*e05-e31*e38*e08+0.5*e01*e302+0.5*e01*e322+1.5*e01*e312+0.5*e01*e342+e31*e30*e00+e31*e32*e02;
        A[3 + 10*17]=e31*e32*e12+e14*e30*e33+e14*e32*e35+e14*e31*e34+e34*e30*e13+e34*e10*e33+e34*e32*e15+e34*e12*e35+e17*e30*e36+e17*e32*e38+e17*e31*e37+e37*e30*e16+e37*e10*e36+e37*e32*e18+e37*e12*e38-e31*e36*e16-e31*e35*e15+0.5*e11*e302+0.5*e11*e322+1.5*e11*e312-e31*e33*e13-e31*e38*e18+0.5*e11*e342+0.5*e11*e372+e31*e30*e10-0.5*e11*e352-0.5*e11*e362-0.5*e11*e332-0.5*e11*e382;
        A[3 + 10*18]=e34*e32*e25+0.5*e21*e342+0.5*e21*e372-0.5*e21*e352-0.5*e21*e362-0.5*e21*e332-0.5*e21*e382+0.5*e21*e302+0.5*e21*e322+1.5*e21*e312+e31*e30*e20+e31*e32*e22+e24*e30*e33+e24*e32*e35+e24*e31*e34+e34*e30*e23+e34*e20*e33+e34*e22*e35+e27*e30*e36+e27*e32*e38+e27*e31*e37+e37*e30*e26+e37*e20*e36+e37*e32*e28+e37*e22*e38-e31*e33*e23-e31*e36*e26-e31*e38*e28-e31*e35*e25;
        A[3 + 10*19]=0.5*e31*e302+0.5*e31*e322+0.5*e313+e34*e30*e33+e34*e32*e35+0.5*e31*e342+e37*e30*e36+e37*e32*e38+0.5*e31*e372-0.5*e31*e352-0.5*e31*e362-0.5*e31*e332-0.5*e31*e382;
        A[4 + 10*0]=e01*e00*e03+0.5*e012*e04+e01*e02*e05+0.5*e04*e032+0.5*e043+0.5*e04*e052+e07*e03*e06+0.5*e04*e072+e07*e05*e08-0.5*e04*e002-0.5*e04*e022-0.5*e04*e062-0.5*e04*e082;
        A[4 + 10*1]=e07*e13*e06+e01*e10*e03-0.5*e14*e002-0.5*e14*e022-0.5*e14*e062-0.5*e14*e082+e01*e00*e13+e01*e11*e04+e01*e12*e05+e01*e02*e15+e11*e00*e03+e11*e02*e05+e04*e13*e03+e04*e15*e05+e07*e03*e16+e07*e04*e17+e07*e15*e08+e07*e05*e18+e17*e03*e06+e17*e05*e08-e04*e10*e00-e04*e12*e02-e04*e16*e06-e04*e18*e08+0.5*e012*e14+1.5*e14*e042+0.5*e14*e032+0.5*e14*e052+0.5*e14*e072;
        A[4 + 10*2]=e11*e10*e03+0.5*e112*e04+0.5*e04*e132+0.5*e04*e152+1.5*e04*e142+0.5*e04*e172-0.5*e04*e102-0.5*e04*e162-0.5*e04*e122-0.5*e04*e182+e01*e10*e13+e01*e12*e15+e01*e11*e14+e11*e00*e13+e11*e12*e05+e11*e02*e15+e14*e13*e03+e14*e15*e05+e07*e13*e16+e07*e15*e18+e07*e14*e17+e17*e13*e06+e17*e03*e16+e17*e15*e08+e17*e05*e18-e14*e10*e00-e14*e12*e02-e14*e16*e06-e14*e18*e08;
        A[4 + 10*3]=e11*e10*e13+e11*e12*e15+0.5*e112*e14+0.5*e14*e132+0.5*e14*e152+0.5*e143+e17*e13*e16+e17*e15*e18+0.5*e14*e172-0.5*e14*e102-0.5*e14*e162-0.5*e14*e122-0.5*e14*e182;
        A[4 + 10*4]=-e04*e28*e08-e04*e26*e06-e04*e22*e02-e04*e20*e00+e27*e05*e08+e27*e03*e06+e07*e05*e28+e07*e25*e08+e07*e04*e27+e07*e03*e26+e07*e23*e06+e04*e25*e05+e04*e23*e03+e21*e02*e05+e21*e00*e03+e01*e02*e25+e01*e22*e05+e01*e21*e04+e01*e00*e23+e01*e20*e03+0.5*e012*e24+0.5*e24*e072+0.5*e24*e052+0.5*e24*e032+1.5*e24*e042-0.5*e24*e022-0.5*e24*e002-0.5*e24*e082-0.5*e24*e062;
        A[4 + 10*5]=e11*e02*e25+e11*e22*e05+e11*e21*e04-e24*e18*e08-e24*e16*e06-e24*e12*e02-e14*e28*e08-e14*e26*e06-e14*e22*e02-e14*e20*e00-e04*e28*e18-e04*e22*e12-e04*e26*e16-e04*e20*e10+e27*e05*e18+e27*e15*e08+e27*e03*e16+e27*e13*e06+e17*e05*e28+e17*e25*e08+e17*e04*e27+e17*e03*e26+e17*e23*e06+e07*e14*e27+e07*e24*e17+e07*e15*e28+e07*e25*e18+e07*e13*e26+e07*e23*e16+e24*e15*e05+e24*e13*e03+e14*e25*e05+e14*e23*e03+3*e04*e24*e14+e04*e25*e15+e04*e23*e13+e21*e02*e15+e21*e12*e05+e21*e00*e13+e21*e10*e03-e24*e10*e00+e01*e20*e13+e01*e10*e23+e01*e22*e15+e01*e12*e25+e01*e11*e24+e11*e20*e03+e11*e00*e23+e01*e21*e14;
        A[4 + 10*6]=e11*e12*e25-0.5*e24*e182-0.5*e24*e102-0.5*e24*e162-0.5*e24*e122+e27*e13*e16+e27*e15*e18-e14*e20*e10-e14*e26*e16-e14*e22*e12-e14*e28*e18+e17*e15*e28+e17*e14*e27+1.5*e24*e142+0.5*e24*e132+0.5*e24*e152+0.5*e112*e24+0.5*e24*e172+e11*e21*e14+e11*e22*e15+e11*e10*e23+e11*e20*e13+e21*e10*e13+e21*e12*e15+e17*e13*e26+e17*e23*e16+e14*e25*e15+e14*e23*e13+e17*e25*e18;
        A[4 + 10*7]=e27*e03*e26+e27*e25*e08+e27*e05*e28-e24*e20*e00-e24*e22*e02-e24*e26*e06-e24*e28*e08+0.5*e04*e232+1.5*e04*e242+0.5*e04*e252+0.5*e04*e272-0.5*e04*e202-0.5*e04*e222-0.5*e04*e262-0.5*e04*e282+e24*e23*e03+e24*e25*e05+e07*e23*e26+e07*e24*e27+e07*e25*e28+e27*e23*e06+e21*e20*e03+e21*e00*e23+e21*e22*e05+e21*e02*e25+0.5*e212*e04+e01*e20*e23+e01*e21*e24+e01*e22*e25;
        A[4 + 10*8]=-e24*e22*e12-e24*e28*e18+0.5*e14*e272-0.5*e14*e202-0.5*e14*e222-0.5*e14*e262-0.5*e14*e282+e17*e23*e26+e17*e24*e27+e17*e25*e28+e27*e23*e16+e27*e13*e26+e27*e25*e18+e27*e15*e28-e24*e20*e10-e24*e26*e16+0.5*e14*e232+1.5*e14*e242+0.5*e14*e252+e21*e10*e23+e21*e22*e15+e21*e12*e25+e24*e23*e13+e24*e25*e15+e21*e20*e13+0.5*e212*e14+e11*e20*e23+e11*e22*e25+e11*e21*e24;
        A[4 + 10*9]=e21*e20*e23+0.5*e212*e24+e21*e22*e25+0.5*e24*e232+0.5*e243+0.5*e24*e252+e27*e23*e26+0.5*e24*e272+e27*e25*e28-0.5*e24*e202-0.5*e24*e222-0.5*e24*e262-0.5*e24*e282;
        A[4 + 10*10]=-e04*e38*e08-e04*e32*e02-e04*e36*e06-0.5*e34*e002-0.5*e34*e022-0.5*e34*e062-0.5*e34*e082+e37*e03*e06+e37*e05*e08-e04*e30*e00+0.5*e34*e032+0.5*e34*e052+0.5*e34*e072+1.5*e34*e042+e01*e30*e03+e01*e00*e33+e01*e31*e04+e01*e32*e05+e01*e02*e35+e31*e00*e03+e31*e02*e05+e04*e33*e03+e04*e35*e05+e07*e03*e36+e07*e04*e37+e07*e35*e08+e07*e05*e38+0.5*e012*e34+e07*e33*e06;
        A[4 + 10*11]=e07*e13*e36+e01*e12*e35-e04*e30*e10+e17*e04*e37+e17*e35*e08+e17*e05*e38+e37*e13*e06+e37*e03*e16+e37*e15*e08+e37*e05*e18-e04*e36*e16+e17*e33*e06+e04*e33*e13+e04*e35*e15+3*e04*e34*e14+e14*e33*e03+e14*e35*e05+e34*e13*e03+e34*e15*e05+e07*e33*e16+e07*e35*e18+e07*e15*e38+e07*e34*e17+e07*e14*e37+e17*e03*e36+e31*e10*e03+e01*e30*e13+e01*e10*e33+e01*e32*e15+e01*e31*e14+e01*e11*e34+e11*e30*e03+e11*e00*e33+e11*e31*e04+e11*e32*e05+e11*e02*e35+e31*e00*e13+e31*e12*e05+e31*e02*e15-e34*e12*e02-e34*e16*e06-e34*e18*e08-e14*e32*e02-e14*e36*e06-e14*e38*e08-e34*e10*e00-e04*e32*e12-e04*e38*e18-e14*e30*e00;
        A[4 + 10*12]=e11*e32*e15-0.5*e34*e102-0.5*e34*e162-0.5*e34*e122-0.5*e34*e182+e37*e13*e16+0.5*e112*e34+1.5*e34*e142+0.5*e34*e132+0.5*e34*e152+0.5*e34*e172+e11*e30*e13+e11*e10*e33+e11*e12*e35+e11*e31*e14+e31*e10*e13+e31*e12*e15+e14*e33*e13+e14*e35*e15+e17*e33*e16+e17*e13*e36+e17*e35*e18+e17*e15*e38+e17*e14*e37+e37*e15*e18-e14*e30*e10-e14*e36*e16-e14*e32*e12-e14*e38*e18;
        A[4 + 10*13]=e01*e22*e35-e04*e30*e20-e04*e32*e22+e01*e31*e24+e01*e21*e34+e01*e32*e25+e21*e30*e03+e21*e00*e33+e21*e31*e04+e21*e32*e05+e21*e02*e35+e31*e20*e03+e31*e00*e23+e31*e22*e05+e31*e02*e25+e04*e33*e23+3*e04*e34*e24+e04*e35*e25+e24*e33*e03+e37*e05*e28-e04*e36*e26-e04*e38*e28-e24*e30*e00-e24*e32*e02-e24*e36*e06-e24*e38*e08+e24*e35*e05+e34*e23*e03+e34*e25*e05+e07*e33*e26+e07*e23*e36+e07*e34*e27+e07*e24*e37+e07*e35*e28+e07*e25*e38+e27*e33*e06+e27*e03*e36+e27*e04*e37+e27*e35*e08+e27*e05*e38+e37*e23*e06+e37*e03*e26+e37*e25*e08-e34*e20*e00-e34*e22*e02-e34*e26*e06-e34*e28*e08+e01*e30*e23+e01*e20*e33;
        A[4 + 10*14]=e21*e10*e33+e11*e30*e23+e11*e20*e33+e11*e31*e24+e11*e21*e34+e11*e32*e25+e11*e22*e35+e21*e30*e13+e21*e32*e15+e21*e12*e35+e21*e31*e14+e31*e20*e13+e31*e10*e23+e31*e22*e15+e31*e12*e25+e14*e33*e23+3*e14*e34*e24+e14*e35*e25+e24*e33*e13+e24*e35*e15+e34*e23*e13+e34*e25*e15+e17*e33*e26+e17*e23*e36+e17*e34*e27+e17*e24*e37+e17*e35*e28+e17*e25*e38+e27*e33*e16+e27*e13*e36+e27*e35*e18+e27*e15*e38+e27*e14*e37+e37*e23*e16+e37*e13*e26+e37*e25*e18+e37*e15*e28-e34*e28*e18-e34*e22*e12-e14*e32*e22-e14*e36*e26-e14*e38*e28-e24*e30*e10-e24*e36*e16-e24*e32*e12-e24*e38*e18-e34*e20*e10-e34*e26*e16-e14*e30*e20;
        A[4 + 10*15]=-0.5*e34*e202-0.5*e34*e222-0.5*e34*e262-0.5*e34*e282+e37*e25*e28-e24*e32*e22-e24*e36*e26-e24*e38*e28-e24*e30*e20+0.5*e212*e34+1.5*e34*e242+0.5*e34*e232+0.5*e34*e252+0.5*e34*e272+e21*e30*e23+e21*e20*e33+e21*e31*e24+e21*e32*e25+e21*e22*e35+e31*e20*e23+e31*e22*e25+e24*e33*e23+e24*e35*e25+e27*e33*e26+e27*e23*e36+e27*e24*e37+e27*e35*e28+e27*e25*e38+e37*e23*e26;
        A[4 + 10*16]=e37*e33*e06+e01*e30*e33+e01*e31*e34+e31*e30*e03+e31*e02*e35+e34*e33*e03+e34*e35*e05+e07*e33*e36+e07*e35*e38+e07*e34*e37+e37*e03*e36+e37*e35*e08+e37*e05*e38-e34*e32*e02-e34*e36*e06-e34*e38*e08+e31*e32*e05+e31*e00*e33+0.5*e312*e04+0.5*e04*e332+0.5*e04*e352+1.5*e04*e342+0.5*e04*e372+e01*e32*e35-0.5*e04*e302-0.5*e04*e322-0.5*e04*e362-0.5*e04*e382-e34*e30*e00;
        A[4 + 10*17]=0.5*e14*e372-0.5*e14*e302-0.5*e14*e322-0.5*e14*e362-0.5*e14*e382+0.5*e312*e14+0.5*e14*e332+0.5*e14*e352+1.5*e14*e342+e11*e30*e33+e11*e32*e35+e11*e31*e34+e31*e30*e13+e31*e10*e33+e31*e32*e15+e31*e12*e35+e34*e33*e13+e34*e35*e15+e17*e33*e36+e17*e35*e38+e17*e34*e37+e37*e33*e16+e37*e13*e36+e37*e35*e18+e37*e15*e38-e34*e30*e10-e34*e36*e16-e34*e32*e12-e34*e38*e18;
        A[4 + 10*18]=-e34*e32*e22-e34*e36*e26-e34*e38*e28+0.5*e24*e332+0.5*e24*e352+1.5*e24*e342+0.5*e24*e372-0.5*e24*e302-0.5*e24*e322-0.5*e24*e362-0.5*e24*e382+e21*e30*e33+0.5*e312*e24+e21*e32*e35+e21*e31*e34+e31*e30*e23+e31*e20*e33+e31*e32*e25+e31*e22*e35+e34*e33*e23+e34*e35*e25+e27*e33*e36+e27*e35*e38+e27*e34*e37+e37*e33*e26+e37*e23*e36+e37*e35*e28+e37*e25*e38-e34*e30*e20;
        A[4 + 10*19]=e31*e30*e33+e31*e32*e35+0.5*e312*e34+0.5*e34*e332+0.5*e34*e352+0.5*e343+e37*e33*e36+e37*e35*e38+0.5*e34*e372-0.5*e34*e302-0.5*e34*e322-0.5*e34*e362-0.5*e34*e382;
        A[5 + 10*0]=e01*e00*e06+0.5*e012*e07+e01*e02*e08+e04*e03*e06+0.5*e042*e07+e04*e05*e08+0.5*e07*e062+0.5*e073+0.5*e07*e082-0.5*e07*e002-0.5*e07*e022-0.5*e07*e032-0.5*e07*e052;
        A[5 + 10*1]=e04*e13*e06+0.5*e042*e17+1.5*e17*e072+0.5*e17*e062+0.5*e17*e082-0.5*e17*e002-0.5*e17*e022-0.5*e17*e032-0.5*e17*e052+e01*e10*e06+e07*e16*e06+e07*e18*e08-e07*e10*e00-e07*e12*e02-e07*e13*e03-e07*e15*e05+e01*e00*e16+e01*e11*e07+e01*e12*e08+e01*e02*e18+e11*e00*e06+e11*e02*e08+e04*e03*e16+e04*e14*e07+e04*e15*e08+e04*e05*e18+e14*e03*e06+e14*e05*e08+0.5*e012*e17;
        A[5 + 10*2]=-e17*e10*e00+0.5*e112*e07+0.5*e142*e07+0.5*e07*e162+0.5*e07*e182+1.5*e07*e172-0.5*e07*e102-0.5*e07*e152-0.5*e07*e132-0.5*e07*e122+e01*e10*e16+e01*e12*e18+e01*e11*e17+e11*e10*e06+e11*e00*e16+e11*e12*e08+e11*e02*e18+e04*e13*e16+e04*e15*e18+e04*e14*e17+e14*e13*e06+e14*e03*e16+e14*e15*e08+e14*e05*e18+e17*e16*e06+e17*e18*e08-e17*e12*e02-e17*e13*e03-e17*e15*e05;
        A[5 + 10*3]=e11*e10*e16+e11*e12*e18+0.5*e112*e17+e14*e13*e16+e14*e15*e18+0.5*e142*e17+0.5*e17*e162+0.5*e17*e182+0.5*e173-0.5*e17*e102-0.5*e17*e152-0.5*e17*e132-0.5*e17*e122;
        A[5 + 10*4]=e01*e22*e08+e07*e28*e08-e07*e20*e00-e07*e23*e03-e07*e22*e02-e07*e25*e05+0.5*e012*e27+0.5*e042*e27+1.5*e27*e072+0.5*e27*e062+0.5*e27*e082-0.5*e27*e002-0.5*e27*e022-0.5*e27*e032-0.5*e27*e052+e07*e26*e06+e01*e20*e06+e01*e00*e26+e01*e21*e07+e01*e02*e28+e21*e00*e06+e21*e02*e08+e04*e23*e06+e04*e03*e26+e04*e24*e07+e04*e25*e08+e04*e05*e28+e24*e03*e06+e24*e05*e08;
        A[5 + 10*5]=e14*e24*e07+e14*e03*e26+e14*e23*e06+e04*e14*e27+e04*e24*e17+e04*e15*e28+e04*e25*e18+e04*e13*e26+e04*e23*e16+e21*e02*e18+e21*e12*e08-e27*e15*e05-e27*e13*e03-e27*e12*e02-e27*e10*e00-e17*e25*e05-e17*e23*e03-e17*e22*e02-e17*e20*e00-e07*e22*e12-e07*e23*e13-e07*e25*e15-e07*e20*e10+e27*e18*e08+e27*e16*e06+e17*e28*e08+e17*e26*e06+3*e07*e27*e17+e07*e28*e18+e07*e26*e16+e24*e05*e18+e24*e15*e08+e24*e03*e16+e24*e13*e06+e14*e05*e28+e14*e25*e08+e01*e12*e28+e01*e20*e16+e01*e10*e26+e01*e22*e18+e01*e21*e17+e11*e20*e06+e01*e11*e27+e21*e00*e16+e21*e10*e06+e11*e21*e07+e11*e22*e08+e11*e02*e28+e11*e00*e26;
        A[5 + 10*6]=-0.5*e27*e102-0.5*e27*e152-0.5*e27*e132-0.5*e27*e122+0.5*e142*e27+1.5*e27*e172+0.5*e27*e162+0.5*e27*e182+0.5*e112*e27+e11*e22*e18+e11*e10*e26+e11*e20*e16-e17*e22*e12-e17*e23*e13-e17*e25*e15-e17*e20*e10+e17*e28*e18+e17*e26*e16+e24*e15*e18+e24*e13*e16+e14*e24*e17+e14*e15*e28+e14*e25*e18+e14*e13*e26+e14*e23*e16+e21*e12*e18+e21*e10*e16+e11*e21*e17+e11*e12*e28;
        A[5 + 10*7]=-0.5*e07*e252+e27*e26*e06+e27*e28*e08-e27*e20*e00-e27*e22*e02-e27*e23*e03-e27*e25*e05+1.5*e07*e272+0.5*e07*e282+e01*e22*e28+e21*e20*e06+e21*e00*e26+e21*e22*e08+e21*e02*e28+e04*e23*e26+e04*e24*e27+e04*e25*e28+e24*e23*e06+e24*e03*e26+e24*e25*e08+e24*e05*e28+0.5*e212*e07+0.5*e242*e07+0.5*e07*e262+e01*e20*e26+e01*e21*e27-0.5*e07*e202-0.5*e07*e232-0.5*e07*e222;
        A[5 + 10*8]=-e27*e25*e15-e27*e23*e13-e27*e22*e12-0.5*e17*e252-0.5*e17*e202-0.5*e17*e222-0.5*e17*e232+0.5*e17*e262+1.5*e17*e272+0.5*e17*e282+e24*e23*e16+e24*e13*e26+e24*e25*e18+e24*e15*e28+e27*e26*e16+e27*e28*e18-e27*e20*e10+e14*e24*e27+e14*e25*e28+0.5*e212*e17+0.5*e242*e17+e11*e20*e26+e11*e21*e27+e11*e22*e28+e21*e20*e16+e21*e10*e26+e21*e22*e18+e21*e12*e28+e14*e23*e26;
        A[5 + 10*9]=e21*e20*e26+0.5*e212*e27+e21*e22*e28+e24*e23*e26+0.5*e242*e27+e24*e25*e28+0.5*e27*e262+0.5*e273+0.5*e27*e282-0.5*e27*e202-0.5*e27*e222-0.5*e27*e232-0.5*e27*e252;
        A[5 + 10*10]=e04*e05*e38+e01*e30*e06-0.5*e37*e002-0.5*e37*e022-0.5*e37*e032-0.5*e37*e052-e07*e32*e02-e07*e35*e05-e07*e33*e03+e07*e36*e06+e07*e38*e08-e07*e30*e00+1.5*e37*e072+0.5*e37*e062+0.5*e37*e082+e01*e02*e38+e31*e00*e06+e31*e02*e08+e04*e33*e06+e04*e03*e36+e04*e34*e07+e04*e35*e08+e34*e03*e06+e34*e05*e08+0.5*e012*e37+0.5*e042*e37+e01*e00*e36+e01*e31*e07+e01*e32*e08;
        A[5 + 10*11]=e14*e33*e06+e11*e30*e06+e11*e00*e36+e11*e31*e07+e31*e10*e06+e11*e32*e08+e11*e02*e38+e31*e00*e16+e31*e12*e08+e31*e02*e18+e04*e33*e16+e04*e13*e36+e04*e35*e18+e04*e15*e38+e01*e10*e36+e01*e32*e18+e01*e12*e38+e01*e31*e17+e01*e11*e37+e01*e30*e16-e17*e35*e05-e37*e10*e00-e37*e12*e02-e37*e13*e03-e37*e15*e05+e37*e18*e08-e07*e30*e10-e07*e35*e15-e07*e33*e13-e07*e32*e12-e17*e30*e00-e17*e32*e02-e17*e33*e03+e07*e38*e18+3*e07*e37*e17+e17*e36*e06+e17*e38*e08+e37*e16*e06+e04*e34*e17+e04*e14*e37+e14*e03*e36+e14*e34*e07+e14*e35*e08+e14*e05*e38+e34*e13*e06+e34*e03*e16+e34*e15*e08+e34*e05*e18+e07*e36*e16;
        A[5 + 10*12]=e11*e32*e18-0.5*e37*e102-0.5*e37*e152-0.5*e37*e132-0.5*e37*e122+0.5*e112*e37+0.5*e142*e37+1.5*e37*e172+0.5*e37*e162+0.5*e37*e182+e11*e10*e36+e11*e12*e38+e11*e31*e17+e31*e10*e16+e31*e12*e18+e14*e33*e16+e14*e13*e36+e14*e35*e18+e14*e15*e38+e14*e34*e17+e34*e13*e16+e34*e15*e18+e17*e36*e16+e17*e38*e18-e17*e30*e10-e17*e35*e15-e17*e33*e13-e17*e32*e12+e11*e30*e16;
        A[5 + 10*13]=e01*e20*e36+e01*e31*e27+e01*e21*e37+e01*e32*e28+e01*e22*e38+e21*e30*e06+e21*e00*e36+e21*e31*e07+e21*e32*e08+e21*e02*e38+e01*e30*e26+e31*e20*e06+e31*e00*e26+e31*e22*e08+e31*e02*e28+e04*e33*e26+e04*e23*e36+e04*e34*e27+e04*e24*e37+e04*e35*e28+e04*e25*e38+e24*e33*e06+e24*e03*e36+e24*e34*e07+e24*e35*e08+e24*e05*e38+e34*e23*e06+e34*e03*e26+e34*e25*e08+e34*e05*e28+e07*e36*e26+3*e07*e37*e27+e07*e38*e28+e27*e36*e06+e27*e38*e08+e37*e26*e06+e37*e28*e08-e07*e30*e20-e07*e32*e22-e07*e33*e23-e07*e35*e25-e27*e30*e00-e27*e32*e02-e27*e33*e03-e27*e35*e05-e37*e20*e00-e37*e22*e02-e37*e23*e03-e37*e25*e05;
        A[5 + 10*14]=e11*e30*e26+e11*e20*e36+e11*e31*e27+e11*e21*e37+e11*e32*e28+e11*e22*e38+e21*e10*e36+e21*e32*e18+e21*e12*e38+e21*e31*e17+e31*e20*e16+e31*e10*e26+e31*e22*e18+e31*e12*e28+e14*e33*e26+e14*e23*e36+e14*e34*e27+e14*e24*e37+e14*e35*e28+e14*e25*e38+e24*e33*e16+e24*e13*e36+e24*e35*e18+e24*e15*e38+e24*e34*e17+e34*e23*e16+e34*e13*e26+e34*e25*e18+e34*e15*e28+e17*e36*e26+3*e17*e37*e27+e17*e38*e28+e27*e36*e16+e27*e38*e18+e37*e26*e16+e37*e28*e18-e17*e30*e20-e17*e32*e22-e17*e33*e23-e17*e35*e25-e27*e30*e10-e27*e35*e15-e27*e33*e13-e27*e32*e12-e37*e20*e10-e37*e25*e15-e37*e23*e13-e37*e22*e12+e21*e30*e16;
        A[5 + 10*15]=e21*e20*e36+e21*e31*e27+e21*e32*e28+e21*e22*e38+e31*e22*e28+e24*e33*e26+e24*e23*e36+e24*e34*e27+e24*e35*e28+e24*e25*e38+e34*e23*e26+e34*e25*e28+e27*e36*e26+e27*e38*e28-e27*e30*e20-e27*e32*e22-e27*e33*e23-e27*e35*e25+0.5*e242*e37+1.5*e37*e272+0.5*e37*e262+0.5*e37*e282+e31*e20*e26+e21*e30*e26+0.5*e212*e37-0.5*e37*e202-0.5*e37*e222-0.5*e37*e232-0.5*e37*e252;
        A[5 + 10*16]=e01*e30*e36+e01*e32*e38+e01*e31*e37+e31*e30*e06+e31*e00*e36+e31*e32*e08+e31*e02*e38+e04*e33*e36+e04*e35*e38+e04*e34*e37+e34*e33*e06+e34*e03*e36+e34*e35*e08+e34*e05*e38+e37*e36*e06+e37*e38*e08-e37*e30*e00-e37*e32*e02-e37*e33*e03-e37*e35*e05+0.5*e312*e07+0.5*e342*e07+0.5*e07*e362+0.5*e07*e382+1.5*e07*e372-0.5*e07*e302-0.5*e07*e352-0.5*e07*e322-0.5*e07*e332;
        A[5 + 10*17]=0.5*e312*e17+0.5*e342*e17+0.5*e17*e362+0.5*e17*e382+1.5*e17*e372-0.5*e17*e302-0.5*e17*e352-0.5*e17*e322-0.5*e17*e332-e37*e32*e12-e37*e33*e13+e11*e30*e36+e11*e32*e38+e11*e31*e37+e31*e30*e16+e31*e10*e36+e31*e32*e18+e31*e12*e38+e14*e33*e36+e14*e35*e38+e14*e34*e37+e34*e33*e16+e34*e13*e36+e34*e35*e18+e34*e15*e38+e37*e36*e16+e37*e38*e18-e37*e30*e10-e37*e35*e15;
        A[5 + 10*18]=e21*e31*e37-0.5*e27*e332+e21*e30*e36+e21*e32*e38+e31*e30*e26+e31*e20*e36+e31*e32*e28+e31*e22*e38+e24*e33*e36+e24*e35*e38+e24*e34*e37+e34*e33*e26+e34*e23*e36+e34*e35*e28+e34*e25*e38+e37*e36*e26+e37*e38*e28-e37*e30*e20-e37*e32*e22-e37*e33*e23-e37*e35*e25+0.5*e312*e27+0.5*e342*e27+0.5*e27*e362+0.5*e27*e382+1.5*e27*e372-0.5*e27*e302-0.5*e27*e352-0.5*e27*e322;
        A[5 + 10*19]=e31*e30*e36+e31*e32*e38+0.5*e312*e37+e34*e33*e36+e34*e35*e38+0.5*e342*e37+0.5*e37*e362+0.5*e37*e382+0.5*e373-0.5*e37*e302-0.5*e37*e352-0.5*e37*e322-0.5*e37*e332;
        A[6 + 10*0]=0.5*e02*e002+0.5*e02*e012+0.5*e023+e05*e00*e03+e05*e01*e04+0.5*e02*e052+e08*e00*e06+e08*e01*e07+0.5*e02*e082-0.5*e02*e032-0.5*e02*e042-0.5*e02*e062-0.5*e02*e072;
        A[6 + 10*1]=-0.5*e12*e042-0.5*e12*e062-0.5*e12*e072+0.5*e12*e082-0.5*e12*e032+1.5*e12*e022+0.5*e12*e002+0.5*e12*e012+0.5*e12*e052+e02*e10*e00+e02*e11*e01+e05*e10*e03+e05*e00*e13+e05*e11*e04+e05*e01*e14+e05*e02*e15+e15*e00*e03+e15*e01*e04+e08*e10*e06+e08*e00*e16+e08*e11*e07+e08*e01*e17+e08*e02*e18+e18*e00*e06+e18*e01*e07-e02*e13*e03-e02*e14*e04-e02*e16*e06-e02*e17*e07;
        A[6 + 10*2]=0.5*e02*e102+1.5*e02*e122+0.5*e02*e112+0.5*e02*e152+0.5*e02*e182-0.5*e02*e162-0.5*e02*e172-0.5*e02*e132-0.5*e02*e142+e12*e10*e00+e12*e11*e01+e05*e10*e13+e05*e12*e15+e05*e11*e14+e15*e10*e03+e15*e00*e13+e15*e11*e04+e15*e01*e14+e08*e10*e16+e08*e12*e18+e08*e11*e17+e18*e10*e06+e18*e00*e16+e18*e11*e07+e18*e01*e17-e12*e13*e03-e12*e14*e04-e12*e16*e06-e12*e17*e07;
        A[6 + 10*3]=0.5*e12*e102+0.5*e123+0.5*e12*e112+e15*e10*e13+0.5*e12*e152+e15*e11*e14+e18*e10*e16+0.5*e12*e182+e18*e11*e17-0.5*e12*e162-0.5*e12*e172-0.5*e12*e132-0.5*e12*e142;
        A[6 + 10*4]=-0.5*e22*e032-0.5*e22*e042-0.5*e22*e062-0.5*e22*e072+0.5*e22*e082+1.5*e22*e022+0.5*e22*e002+0.5*e22*e012+0.5*e22*e052+e02*e20*e00+e02*e21*e01+e05*e20*e03+e05*e00*e23+e05*e21*e04+e05*e01*e24+e05*e02*e25+e25*e00*e03+e25*e01*e04+e08*e20*e06+e08*e00*e26+e08*e21*e07+e08*e01*e27+e08*e02*e28+e28*e00*e06+e28*e01*e07-e02*e27*e07-e02*e23*e03-e02*e24*e04-e02*e26*e06;
        A[6 + 10*5]=-e22*e17*e07-e22*e16*e06-e22*e14*e04-e22*e13*e03-e12*e26*e06-e12*e24*e04-e12*e23*e03-e12*e27*e07-e02*e24*e14-e02*e23*e13-e02*e27*e17-e02*e26*e16+e28*e01*e17+e28*e11*e07+e28*e00*e16+e28*e10*e06+e18*e02*e28+e18*e01*e27+e18*e21*e07+e18*e00*e26+e18*e20*e06+e08*e11*e27+e08*e21*e17+e08*e12*e28+e08*e22*e18+e08*e10*e26+e25*e01*e14+e25*e11*e04+e25*e00*e13+e25*e10*e03+e15*e01*e24+e02*e21*e11+e12*e21*e01+e15*e02*e25+e15*e21*e04+e05*e22*e15+e05*e11*e24+e15*e20*e03+e15*e00*e23+e05*e10*e23+e05*e12*e25+e05*e21*e14+e22*e10*e00+e22*e11*e01+e02*e20*e10+3*e02*e22*e12+e12*e20*e00+e08*e20*e16+e05*e20*e13;
        A[6 + 10*6]=-e12*e24*e14-e12*e23*e13-e12*e27*e17-e12*e26*e16+e28*e11*e17+e28*e10*e16+e18*e11*e27+e18*e21*e17+e18*e12*e28+e18*e10*e26+e18*e20*e16+e25*e11*e14+e25*e10*e13+e15*e11*e24+e15*e21*e14+e15*e12*e25+e15*e10*e23+e15*e20*e13+e12*e21*e11+0.5*e22*e182+0.5*e22*e152+1.5*e22*e122+0.5*e22*e102+e12*e20*e10+0.5*e22*e112-0.5*e22*e172-0.5*e22*e132-0.5*e22*e142-0.5*e22*e162;
        A[6 + 10*7]=0.5*e02*e282+e28*e01*e27-e22*e27*e07-e22*e23*e03-e22*e24*e04-e22*e26*e06+0.5*e02*e252+e05*e20*e23+e05*e22*e25+e25*e20*e03+e25*e00*e23+e25*e21*e04+e25*e01*e24+e08*e20*e26+e08*e21*e27+e08*e22*e28+e28*e20*e06+e28*e00*e26+e28*e21*e07+e05*e21*e24+0.5*e02*e202+0.5*e02*e212+1.5*e02*e222+e22*e20*e00+e22*e21*e01-0.5*e02*e272-0.5*e02*e242-0.5*e02*e232-0.5*e02*e262;
        A[6 + 10*8]=-e22*e27*e17-e22*e23*e13-e22*e24*e14-0.5*e12*e232-0.5*e12*e262-0.5*e12*e242-0.5*e12*e272+0.5*e12*e282+e18*e21*e27+e28*e20*e16+e28*e10*e26+e28*e21*e17+e28*e11*e27-e22*e26*e16+e18*e22*e28+0.5*e12*e252+0.5*e12*e202+0.5*e12*e212+1.5*e12*e222+e22*e20*e10+e15*e20*e23+e15*e21*e24+e15*e22*e25+e25*e20*e13+e25*e10*e23+e25*e21*e14+e25*e11*e24+e18*e20*e26+e22*e21*e11;
        A[6 + 10*9]=0.5*e22*e202+0.5*e22*e212+0.5*e223+e25*e20*e23+e25*e21*e24+0.5*e22*e252+e28*e20*e26+e28*e21*e27+0.5*e22*e282-0.5*e22*e232-0.5*e22*e262-0.5*e22*e242-0.5*e22*e272;
        A[6 + 10*10]=e08*e31*e07-0.5*e32*e032-e02*e33*e03-e02*e34*e04-e02*e36*e06-0.5*e32*e042-0.5*e32*e062-0.5*e32*e072+e38*e01*e07+e38*e00*e06-e02*e37*e07+e05*e31*e04+e05*e01*e34+e05*e02*e35+e35*e01*e04+e35*e00*e03+e08*e30*e06+e08*e00*e36+e08*e01*e37+e08*e02*e38+0.5*e32*e052+e02*e30*e00+e02*e31*e01+e05*e30*e03+e05*e00*e33+1.5*e32*e022+0.5*e32*e012+0.5*e32*e002+0.5*e32*e082;
        A[6 + 10*11]=e05*e32*e15+e32*e11*e01+e38*e10*e06+e08*e12*e38-e32*e14*e04-e32*e16*e06-e32*e17*e07-e12*e36*e06-e32*e13*e03-e02*e34*e14-e12*e37*e07-e12*e33*e03-e12*e34*e04-e02*e37*e17-e02*e33*e13+e38*e01*e17-e02*e36*e16+e18*e01*e37+e18*e02*e38+e38*e00*e16+e38*e11*e07+e08*e30*e16+e08*e10*e36+e08*e32*e18+e08*e31*e17+e08*e11*e37+e18*e30*e06+e18*e00*e36+e18*e31*e07+e35*e10*e03+e35*e00*e13+e35*e11*e04+e35*e01*e14+e15*e02*e35+e05*e10*e33+e05*e12*e35+e05*e31*e14+e05*e11*e34+e15*e30*e03+e15*e00*e33+e15*e31*e04+e15*e01*e34+e05*e30*e13+e02*e30*e10+e02*e31*e11+3*e02*e32*e12+e12*e30*e00+e12*e31*e01+e32*e10*e00;
        A[6 + 10*12]=0.5*e32*e102+0.5*e32*e112+e12*e30*e10+1.5*e32*e122+e12*e31*e11+e15*e30*e13+e15*e10*e33+e15*e12*e35+e15*e31*e14+e15*e11*e34+e35*e10*e13-0.5*e32*e162-0.5*e32*e172-0.5*e32*e132-0.5*e32*e142-e12*e37*e17-e12*e33*e13-e12*e34*e14+0.5*e32*e182+0.5*e32*e152+e35*e11*e14+e18*e30*e16+e18*e10*e36+e18*e12*e38+e18*e31*e17+e18*e11*e37+e38*e10*e16+e38*e11*e17-e12*e36*e16;
        A[6 + 10*13]=3*e02*e32*e22+e05*e31*e24+e08*e22*e38+e02*e31*e21+e22*e30*e00+e22*e31*e01+e32*e20*e00+e32*e21*e01+e05*e30*e23+e05*e20*e33+e05*e21*e34-e22*e37*e07-e22*e33*e03-e22*e34*e04-e22*e36*e06-e32*e27*e07-e32*e23*e03-e32*e24*e04-e32*e26*e06+e05*e32*e25+e25*e30*e03+e25*e00*e33+e25*e31*e04+e25*e01*e34+e25*e02*e35+e35*e20*e03+e35*e00*e23+e35*e21*e04+e35*e01*e24+e08*e30*e26+e08*e20*e36+e08*e31*e27+e08*e21*e37+e08*e32*e28+e28*e30*e06+e28*e00*e36+e28*e31*e07+e28*e01*e37+e28*e02*e38+e38*e20*e06+e38*e00*e26+e38*e21*e07+e38*e01*e27-e02*e33*e23-e02*e36*e26-e02*e34*e24-e02*e37*e27+e05*e22*e35+e02*e30*e20;
        A[6 + 10*14]=e18*e22*e38+e12*e31*e21+3*e12*e32*e22+e22*e30*e10+e22*e31*e11+e32*e20*e10+e32*e21*e11+e15*e30*e23+e15*e20*e33+e15*e31*e24+e15*e21*e34+e15*e32*e25+e15*e22*e35+e25*e30*e13+e25*e10*e33+e25*e12*e35+e25*e31*e14+e25*e11*e34+e35*e20*e13+e35*e10*e23+e35*e21*e14+e35*e11*e24+e18*e30*e26+e18*e20*e36+e18*e31*e27+e18*e21*e37+e18*e32*e28+e28*e30*e16+e28*e10*e36+e28*e12*e38+e28*e31*e17+e28*e11*e37+e38*e20*e16+e38*e10*e26+e12*e30*e20-e22*e37*e17-e22*e33*e13-e22*e34*e14-e32*e26*e16-e32*e27*e17-e32*e23*e13-e32*e24*e14-e22*e36*e16+e38*e21*e17+e38*e11*e27-e12*e33*e23-e12*e36*e26-e12*e34*e24-e12*e37*e27;
        A[6 + 10*15]=e25*e30*e23+e22*e30*e20+e22*e31*e21+e25*e20*e33+e25*e31*e24+e25*e21*e34+e25*e22*e35+e35*e20*e23+e35*e21*e24+e28*e30*e26+e28*e20*e36+e28*e31*e27+e28*e21*e37+e28*e22*e38+e38*e20*e26+e38*e21*e27-e22*e33*e23-e22*e36*e26-e22*e34*e24-e22*e37*e27+0.5*e32*e212+0.5*e32*e252+1.5*e32*e222+0.5*e32*e202+0.5*e32*e282-0.5*e32*e232-0.5*e32*e262-0.5*e32*e242-0.5*e32*e272;
        A[6 + 10*16]=0.5*e02*e302+1.5*e02*e322+0.5*e02*e312+0.5*e02*e352+0.5*e02*e382-0.5*e02*e342-0.5*e02*e362-0.5*e02*e332-0.5*e02*e372+e38*e30*e06+e32*e30*e00+e32*e31*e01+e05*e30*e33+e05*e32*e35+e05*e31*e34+e35*e30*e03+e35*e00*e33+e35*e31*e04+e35*e01*e34+e08*e30*e36+e08*e32*e38+e08*e31*e37+e38*e00*e36+e38*e31*e07+e38*e01*e37-e32*e37*e07-e32*e33*e03-e32*e34*e04-e32*e36*e06;
        A[6 + 10*17]=e32*e30*e10+e32*e31*e11+e15*e30*e33+e15*e32*e35+e15*e31*e34+e35*e30*e13+e35*e10*e33+e35*e31*e14+e35*e11*e34+e18*e30*e36+e18*e32*e38+e18*e31*e37+e38*e30*e16+e38*e10*e36+e38*e31*e17+e38*e11*e37-e32*e36*e16-e32*e37*e17-e32*e33*e13-e32*e34*e14+0.5*e12*e382-0.5*e12*e342-0.5*e12*e362-0.5*e12*e332-0.5*e12*e372+0.5*e12*e352+1.5*e12*e322+0.5*e12*e312+0.5*e12*e302;
        A[6 + 10*18]=0.5*e22*e302+0.5*e22*e312+0.5*e22*e352+0.5*e22*e382-0.5*e22*e342-0.5*e22*e362-0.5*e22*e332-0.5*e22*e372+1.5*e22*e322+e32*e30*e20+e32*e31*e21+e25*e30*e33+e25*e32*e35+e25*e31*e34+e35*e30*e23+e35*e20*e33+e35*e31*e24+e35*e21*e34+e28*e30*e36+e28*e32*e38+e28*e31*e37+e38*e30*e26+e38*e20*e36+e38*e31*e27+e38*e21*e37-e32*e33*e23-e32*e36*e26-e32*e34*e24-e32*e37*e27;
        A[6 + 10*19]=0.5*e32*e302+0.5*e323+0.5*e32*e312+e35*e30*e33+0.5*e32*e352+e35*e31*e34+e38*e30*e36+0.5*e32*e382+e38*e31*e37-0.5*e32*e342-0.5*e32*e362-0.5*e32*e332-0.5*e32*e372;
        A[7 + 10*0]=e02*e01*e04+e02*e00*e03+0.5*e022*e05+0.5*e05*e032+0.5*e05*e042+0.5*e053+e08*e03*e06+e08*e04*e07+0.5*e05*e082-0.5*e05*e002-0.5*e05*e062-0.5*e05*e012-0.5*e05*e072;
        A[7 + 10*1]=e08*e13*e06+e02*e10*e03+e02*e00*e13+e02*e11*e04+e02*e01*e14+e02*e12*e05+e12*e01*e04+e12*e00*e03+e05*e13*e03+e05*e14*e04+e08*e03*e16+e08*e14*e07+e08*e04*e17+e08*e05*e18+e18*e03*e06+e18*e04*e07-e05*e10*e00-e05*e11*e01-e05*e16*e06-e05*e17*e07+0.5*e022*e15+1.5*e15*e052+0.5*e15*e032+0.5*e15*e042+0.5*e15*e082-0.5*e15*e002-0.5*e15*e062-0.5*e15*e012-0.5*e15*e072;
        A[7 + 10*2]=0.5*e122*e05+0.5*e05*e132+1.5*e05*e152+0.5*e05*e142+0.5*e05*e182-0.5*e05*e102-0.5*e05*e162-0.5*e05*e112-0.5*e05*e172+e02*e10*e13+e02*e12*e15+e02*e11*e14+e12*e10*e03+e12*e00*e13+e12*e11*e04+e12*e01*e14+e15*e13*e03+e15*e14*e04+e08*e13*e16+e08*e15*e18+e08*e14*e17+e18*e13*e06+e18*e03*e16+e18*e14*e07+e18*e04*e17-e15*e11*e01-e15*e16*e06-e15*e17*e07-e15*e10*e00;
        A[7 + 10*3]=e12*e10*e13+0.5*e122*e15+e12*e11*e14+0.5*e15*e132+0.5*e153+0.5*e15*e142+e18*e13*e16+0.5*e15*e182+e18*e14*e17-0.5*e15*e102-0.5*e15*e162-0.5*e15*e112-0.5*e15*e172;
        A[7 + 10*4]=0.5*e25*e082-0.5*e25*e002-0.5*e25*e062-0.5*e25*e012-0.5*e25*e072+e02*e20*e03+e02*e00*e23+e02*e21*e04+e02*e01*e24+e02*e22*e05+e22*e01*e04+e22*e00*e03+e05*e23*e03+e05*e24*e04+e08*e23*e06+e08*e03*e26+e08*e24*e07+e08*e04*e27+e08*e05*e28+e28*e03*e06+e28*e04*e07-e05*e20*e00-e05*e27*e07-e05*e21*e01-e05*e26*e06+0.5*e022*e25+1.5*e25*e052+0.5*e25*e032+0.5*e25*e042;
        A[7 + 10*5]=-e25*e17*e07-e25*e16*e06-e25*e11*e01-e25*e10*e00-e15*e26*e06-e15*e21*e01-e15*e27*e07-e15*e20*e00-e05*e27*e17-e05*e21*e11-e05*e26*e16-e05*e20*e10+e28*e04*e17+e28*e14*e07+e28*e03*e16+e28*e13*e06+e18*e05*e28+e18*e04*e27+e18*e24*e07+e18*e03*e26+e18*e23*e06+e08*e14*e27+e08*e24*e17+e08*e15*e28+e08*e25*e18+e08*e13*e26+e08*e23*e16+e25*e14*e04+e25*e13*e03+e15*e24*e04+e15*e23*e03+e05*e24*e14+3*e05*e25*e15+e05*e23*e13+e22*e01*e14+e22*e11*e04+e22*e00*e13+e22*e10*e03+e12*e22*e05+e12*e01*e24+e12*e21*e04+e12*e00*e23+e12*e20*e03+e02*e11*e24+e02*e21*e14+e02*e12*e25+e02*e22*e15+e02*e10*e23+e02*e20*e13;
        A[7 + 10*6]=-e15*e27*e17-e15*e21*e11-e15*e26*e16+e28*e14*e17+e28*e13*e16+e18*e14*e27+e18*e24*e17+e18*e15*e28+e18*e13*e26+e15*e24*e14+e15*e23*e13+e22*e11*e14+e22*e10*e13+e12*e11*e24+e12*e21*e14+e12*e22*e15+e12*e10*e23+e18*e23*e16+0.5*e25*e142+0.5*e25*e182+1.5*e25*e152+0.5*e25*e132+0.5*e122*e25+e12*e20*e13-0.5*e25*e172-0.5*e25*e162-0.5*e25*e112-0.5*e25*e102-e15*e20*e10;
        A[7 + 10*7]=e28*e24*e07-0.5*e05*e272-0.5*e05*e262-0.5*e05*e212+0.5*e05*e282-0.5*e05*e202+e28*e23*e06+e08*e23*e26+e08*e25*e28+e08*e24*e27+e28*e03*e26+e28*e04*e27-e25*e27*e07-e25*e21*e01-e25*e26*e06+e02*e20*e23+e02*e22*e25+e02*e21*e24+e22*e20*e03+e22*e00*e23+e22*e21*e04+e22*e01*e24+e25*e23*e03+e25*e24*e04+0.5*e222*e05+0.5*e05*e232+1.5*e05*e252+0.5*e05*e242-e25*e20*e00;
        A[7 + 10*8]=-0.5*e15*e202-0.5*e15*e262-0.5*e15*e212-0.5*e15*e272+e18*e23*e26+e18*e25*e28+e18*e24*e27+e28*e23*e16+e28*e13*e26+e28*e24*e17+e28*e14*e27-e25*e20*e10-e25*e26*e16-e25*e21*e11-e25*e27*e17+0.5*e15*e282+0.5*e15*e232+1.5*e15*e252+0.5*e15*e242+0.5*e222*e15+e12*e21*e24+e22*e20*e13+e22*e10*e23+e22*e21*e14+e22*e11*e24+e25*e23*e13+e25*e24*e14+e12*e20*e23+e12*e22*e25;
        A[7 + 10*9]=e22*e20*e23+0.5*e222*e25+e22*e21*e24+0.5*e25*e232+0.5*e253+0.5*e25*e242+e28*e23*e26+0.5*e25*e282+e28*e24*e27-0.5*e25*e202-0.5*e25*e262-0.5*e25*e212-0.5*e25*e272;
        A[7 + 10*10]=-0.5*e35*e062-0.5*e35*e012-0.5*e35*e072-e05*e30*e00-e05*e31*e01-e05*e36*e06-e05*e37*e07-0.5*e35*e002+0.5*e35*e082+e05*e34*e04+e08*e33*e06+e08*e03*e36+e08*e34*e07+e08*e04*e37+e08*e05*e38+e38*e04*e07+e38*e03*e06+0.5*e022*e35+1.5*e35*e052+0.5*e35*e042+0.5*e35*e032+e02*e30*e03+e02*e00*e33+e02*e31*e04+e02*e01*e34+e02*e32*e05+e32*e01*e04+e32*e00*e03+e05*e33*e03;
        A[7 + 10*11]=e08*e33*e16-e35*e16*e06-e35*e17*e07-e15*e30*e00-e15*e37*e07-e15*e31*e01-e15*e36*e06-e35*e10*e00-e35*e11*e01-e05*e37*e17-e05*e31*e11+e38*e04*e17-e05*e30*e10-e05*e36*e16+e18*e33*e06+e18*e03*e36+e18*e34*e07+e18*e04*e37+e18*e05*e38+e38*e13*e06+e38*e03*e16+e38*e14*e07+e35*e14*e04+e08*e13*e36+e08*e35*e18+e08*e15*e38+e08*e34*e17+e08*e14*e37+e35*e13*e03+e05*e33*e13+3*e05*e35*e15+e05*e34*e14+e15*e33*e03+e15*e34*e04+e12*e01*e34+e12*e32*e05+e32*e10*e03+e32*e00*e13+e32*e11*e04+e32*e01*e14+e12*e30*e03+e02*e30*e13+e02*e32*e15+e02*e10*e33+e02*e12*e35+e12*e00*e33+e02*e31*e14+e02*e11*e34+e12*e31*e04;
        A[7 + 10*12]=-0.5*e35*e162-0.5*e35*e172-e15*e36*e16-e15*e31*e11-e15*e37*e17-0.5*e35*e102-0.5*e35*e112-e15*e30*e10+e18*e13*e36+e18*e15*e38+e18*e34*e17+e18*e14*e37+e38*e13*e16+e38*e14*e17+e18*e33*e16+1.5*e35*e152+0.5*e35*e132+0.5*e35*e142+0.5*e35*e182+0.5*e122*e35+e32*e10*e13+e32*e11*e14+e15*e33*e13+e15*e34*e14+e12*e10*e33+e12*e32*e15+e12*e31*e14+e12*e11*e34+e12*e30*e13;
        A[7 + 10*13]=e05*e33*e23+3*e05*e35*e25+e05*e34*e24+e25*e33*e03+e25*e34*e04+e35*e23*e03+e35*e24*e04+e08*e33*e26+e08*e23*e36+e08*e35*e28+e02*e20*e33+e02*e32*e25+e02*e22*e35+e02*e31*e24+e02*e21*e34+e22*e30*e03+e22*e00*e33+e22*e31*e04+e22*e01*e34+e22*e32*e05+e32*e20*e03+e32*e00*e23+e32*e21*e04+e32*e01*e24+e02*e30*e23-e35*e27*e07-e35*e21*e01-e35*e26*e06+e08*e25*e38+e08*e34*e27+e08*e24*e37+e28*e33*e06+e28*e03*e36+e28*e34*e07+e28*e04*e37+e28*e05*e38+e38*e23*e06+e38*e03*e26+e38*e24*e07+e38*e04*e27-e05*e30*e20-e05*e36*e26-e05*e31*e21-e05*e37*e27-e25*e30*e00-e25*e37*e07-e25*e31*e01-e25*e36*e06-e35*e20*e00;
        A[7 + 10*14]=e12*e21*e34+e18*e25*e38+e12*e30*e23+e12*e20*e33+e12*e32*e25+e12*e22*e35+e12*e31*e24+e22*e30*e13+e22*e10*e33+e22*e32*e15+e22*e31*e14+e22*e11*e34+e32*e20*e13+e32*e10*e23+e32*e21*e14-e25*e30*e10-e25*e36*e16-e25*e31*e11-e25*e37*e17-e35*e20*e10-e35*e26*e16-e35*e21*e11-e35*e27*e17+e15*e33*e23+3*e15*e35*e25+e15*e34*e24+e25*e33*e13+e25*e34*e14+e35*e23*e13+e35*e24*e14+e18*e33*e26+e18*e23*e36+e18*e35*e28+e18*e34*e27+e18*e24*e37+e28*e33*e16+e28*e13*e36+e28*e15*e38+e28*e34*e17+e28*e14*e37+e38*e23*e16+e38*e13*e26+e38*e24*e17+e38*e14*e27-e15*e30*e20-e15*e36*e26-e15*e31*e21-e15*e37*e27+e32*e11*e24;
        A[7 + 10*15]=-0.5*e35*e202-0.5*e35*e262-0.5*e35*e212-0.5*e35*e272+e25*e34*e24+e28*e23*e36+e28*e25*e38+e28*e34*e27+e28*e24*e37+e38*e23*e26+e38*e24*e27-e25*e30*e20-e25*e36*e26-e25*e31*e21-e25*e37*e27+e25*e33*e23+0.5*e222*e35+1.5*e35*e252+0.5*e35*e232+0.5*e35*e242+0.5*e35*e282+e22*e30*e23+e22*e20*e33+e22*e32*e25+e22*e31*e24+e22*e21*e34+e32*e20*e23+e32*e21*e24+e28*e33*e26;
        A[7 + 10*16]=-e35*e30*e00-e35*e31*e01-e35*e36*e06-e35*e37*e07+0.5*e322*e05+0.5*e05*e332+0.5*e05*e342+1.5*e05*e352+0.5*e05*e382-0.5*e05*e302-0.5*e05*e362-0.5*e05*e312-0.5*e05*e372+e02*e30*e33+e02*e31*e34+e02*e32*e35+e32*e30*e03+e32*e00*e33+e32*e31*e04+e32*e01*e34+e35*e33*e03+e35*e34*e04+e08*e33*e36+e08*e34*e37+e08*e35*e38+e38*e33*e06+e38*e03*e36+e38*e34*e07+e38*e04*e37;
        A[7 + 10*17]=-e35*e30*e10+e12*e32*e35-0.5*e15*e362-0.5*e15*e312-0.5*e15*e372-e35*e36*e16+0.5*e322*e15+0.5*e15*e332+0.5*e15*e342+1.5*e15*e352+0.5*e15*e382-0.5*e15*e302+e12*e30*e33+e12*e31*e34+e32*e30*e13+e32*e10*e33+e32*e31*e14+e32*e11*e34+e35*e33*e13+e35*e34*e14+e18*e33*e36+e18*e34*e37+e18*e35*e38+e38*e33*e16+e38*e13*e36+e38*e34*e17+e38*e14*e37-e35*e31*e11-e35*e37*e17;
        A[7 + 10*18]=-0.5*e25*e302-0.5*e25*e362-0.5*e25*e312-0.5*e25*e372+0.5*e322*e25+0.5*e25*e332+0.5*e25*e342+1.5*e25*e352+0.5*e25*e382+e22*e30*e33+e22*e31*e34+e22*e32*e35+e32*e30*e23+e32*e20*e33+e32*e31*e24+e32*e21*e34+e35*e33*e23+e35*e34*e24+e28*e33*e36+e28*e34*e37+e28*e35*e38+e38*e33*e26+e38*e23*e36+e38*e34*e27+e38*e24*e37-e35*e30*e20-e35*e36*e26-e35*e31*e21-e35*e37*e27;
        A[7 + 10*19]=e32*e30*e33+e32*e31*e34+0.5*e322*e35+0.5*e35*e332+0.5*e35*e342+0.5*e353+e38*e33*e36+e38*e34*e37+0.5*e35*e382-0.5*e35*e302-0.5*e35*e362-0.5*e35*e312-0.5*e35*e372;
        A[8 + 10*0]=e02*e00*e06+e02*e01*e07+0.5*e022*e08+e05*e04*e07+e05*e03*e06+0.5*e052*e08+0.5*e08*e062+0.5*e08*e072+0.5*e083-0.5*e08*e042-0.5*e08*e002-0.5*e08*e012-0.5*e08*e032;
        A[8 + 10*1]=e02*e10*e06+e02*e00*e16+e02*e11*e07+e02*e01*e17+e02*e12*e08+e12*e00*e06+e12*e01*e07+e05*e13*e06+e05*e03*e16+e05*e14*e07+e05*e04*e17+e05*e15*e08+e15*e04*e07+e15*e03*e06+e08*e16*e06+e08*e17*e07-e08*e10*e00-e08*e11*e01-e08*e13*e03-e08*e14*e04+0.5*e022*e18+0.5*e052*e18+1.5*e18*e082+0.5*e18*e062+0.5*e18*e072-0.5*e18*e042-0.5*e18*e002-0.5*e18*e012-0.5*e18*e032;
        A[8 + 10*2]=e12*e01*e17+0.5*e152*e08+0.5*e08*e162+1.5*e08*e182+0.5*e08*e172-0.5*e08*e102-0.5*e08*e112-0.5*e08*e132-0.5*e08*e142+e05*e13*e16+e05*e14*e17+e05*e15*e18+e15*e13*e06+e15*e03*e16+e15*e14*e07+e15*e04*e17+e18*e16*e06+e18*e17*e07-e18*e10*e00-e18*e11*e01-e18*e13*e03-e18*e14*e04+0.5*e122*e08+e02*e10*e16+e02*e12*e18+e02*e11*e17+e12*e10*e06+e12*e00*e16+e12*e11*e07;
        A[8 + 10*3]=e12*e10*e16+0.5*e122*e18+e12*e11*e17+e15*e13*e16+e15*e14*e17+0.5*e152*e18+0.5*e18*e162+0.5*e183+0.5*e18*e172-0.5*e18*e102-0.5*e18*e112-0.5*e18*e132-0.5*e18*e142;
        A[8 + 10*4]=-e08*e20*e00+e08*e27*e07-e08*e21*e01-e08*e23*e03-e08*e24*e04+e02*e20*e06+e02*e00*e26+e02*e21*e07+e02*e01*e27+e02*e22*e08+e22*e00*e06+e22*e01*e07+e05*e23*e06+e05*e03*e26+e05*e24*e07+e05*e04*e27+e05*e25*e08+e25*e04*e07+e25*e03*e06+e08*e26*e06+0.5*e022*e28+0.5*e052*e28+1.5*e28*e082+0.5*e28*e062+0.5*e28*e072-0.5*e28*e042-0.5*e28*e002-0.5*e28*e012-0.5*e28*e032;
        A[8 + 10*5]=e22*e10*e06+e22*e11*e07+e22*e01*e17+e05*e23*e16+e05*e13*e26+e05*e25*e18+e05*e15*e28+e05*e24*e17+e05*e14*e27+e15*e23*e06+e15*e03*e26+e15*e24*e07+e15*e04*e27+e15*e25*e08+e25*e13*e06+e25*e03*e16+e25*e14*e07+e25*e04*e17+e08*e26*e16+3*e08*e28*e18+e08*e27*e17+e18*e26*e06+e18*e27*e07+e22*e00*e16+e28*e16*e06+e28*e17*e07-e08*e20*e10-e08*e21*e11-e08*e23*e13-e08*e24*e14-e18*e20*e00-e18*e21*e01-e18*e23*e03-e18*e24*e04-e28*e10*e00-e28*e11*e01-e28*e13*e03-e28*e14*e04+e02*e20*e16+e02*e10*e26+e02*e22*e18+e02*e12*e28+e02*e21*e17+e02*e11*e27+e12*e20*e06+e12*e00*e26+e12*e21*e07+e12*e01*e27+e12*e22*e08;
        A[8 + 10*6]=-e18*e24*e14-e18*e21*e11-e18*e23*e13-e18*e20*e10+e18*e27*e17+e18*e26*e16+e25*e14*e17+e25*e13*e16+e15*e25*e18+e15*e14*e27+e15*e24*e17+e15*e13*e26+e15*e23*e16+e22*e11*e17+e22*e10*e16+e12*e11*e27+e12*e21*e17+e12*e22*e18+e12*e10*e26+e12*e20*e16+0.5*e28*e162+0.5*e28*e172+1.5*e28*e182+0.5*e152*e28-0.5*e28*e142-0.5*e28*e112-0.5*e28*e132-0.5*e28*e102+0.5*e122*e28;
        A[8 + 10*7]=-e28*e24*e04-e28*e21*e01-e28*e23*e03-e28*e20*e00+e28*e27*e07+e28*e26*e06+e25*e04*e27+e25*e24*e07+e25*e03*e26+e05*e24*e27+e05*e25*e28+e05*e23*e26+e22*e01*e27+e22*e21*e07+e22*e00*e26+e22*e20*e06+e02*e22*e28+e02*e20*e26+e02*e21*e27+0.5*e222*e08-0.5*e08*e242-0.5*e08*e212-0.5*e08*e232-0.5*e08*e202+0.5*e08*e262+0.5*e08*e272+1.5*e08*e282+0.5*e252*e08+e25*e23*e06;
        A[8 + 10*8]=e25*e24*e17+e25*e14*e27+e28*e26*e16+e28*e27*e17-e28*e21*e11-e28*e24*e14+e12*e22*e28+e22*e10*e26+e22*e21*e17+e22*e11*e27+e15*e23*e26+e15*e25*e28+e15*e24*e27+e25*e23*e16+e25*e13*e26+e22*e20*e16+0.5*e222*e18+0.5*e252*e18+0.5*e18*e262+0.5*e18*e272+e12*e20*e26+e12*e21*e27-e28*e20*e10-0.5*e18*e232-0.5*e18*e242-e28*e23*e13-0.5*e18*e212+1.5*e18*e282-0.5*e18*e202;
        A[8 + 10*9]=e22*e20*e26+e22*e21*e27+0.5*e222*e28+e25*e23*e26+0.5*e252*e28+e25*e24*e27+0.5*e28*e262+0.5*e28*e272+0.5*e283-0.5*e28*e202-0.5*e28*e212-0.5*e28*e232-0.5*e28*e242;
        A[8 + 10*10]=-e08*e30*e00-0.5*e38*e042-0.5*e38*e002-0.5*e38*e012-0.5*e38*e032+1.5*e38*e082+0.5*e38*e062+0.5*e38*e072+e32*e01*e07+e05*e33*e06+e05*e03*e36+e05*e34*e07+e05*e04*e37+e05*e35*e08+e35*e04*e07+e35*e03*e06+e08*e36*e06+e08*e37*e07+0.5*e052*e38+e32*e00*e06+e02*e30*e06+e02*e00*e36+e02*e31*e07+e02*e01*e37+e02*e32*e08+0.5*e022*e38-e08*e33*e03-e08*e31*e01-e08*e34*e04;
        A[8 + 10*11]=-e38*e11*e01-e38*e14*e04-e38*e10*e00-e38*e13*e03-e18*e30*e00-e18*e33*e03-e18*e31*e01-e18*e34*e04-e08*e30*e10-e08*e33*e13-e08*e31*e11-e08*e34*e14+3*e08*e38*e18+e08*e37*e17+e18*e36*e06+e18*e37*e07+e38*e16*e06+e38*e17*e07+e15*e35*e08+e35*e13*e06+e35*e03*e16+e35*e14*e07+e35*e04*e17+e08*e36*e16+e05*e35*e18+e05*e15*e38+e15*e33*e06+e15*e03*e36+e15*e34*e07+e15*e04*e37+e05*e14*e37+e12*e30*e06+e12*e31*e07+e12*e01*e37+e12*e00*e36+e12*e32*e08+e32*e10*e06+e32*e00*e16+e32*e11*e07+e32*e01*e17+e05*e33*e16+e05*e13*e36+e05*e34*e17+e02*e30*e16+e02*e10*e36+e02*e32*e18+e02*e12*e38+e02*e31*e17+e02*e11*e37;
        A[8 + 10*12]=e12*e30*e16+e12*e10*e36+e12*e32*e18+e12*e31*e17+e12*e11*e37+e32*e10*e16+e32*e11*e17+e15*e33*e16+e15*e13*e36-0.5*e38*e102-0.5*e38*e112-0.5*e38*e132-0.5*e38*e142+0.5*e38*e162+0.5*e38*e172+e15*e34*e17+e15*e14*e37+e15*e35*e18+e35*e13*e16+e35*e14*e17+e18*e36*e16+e18*e37*e17-e18*e30*e10-e18*e33*e13-e18*e31*e11-e18*e34*e14+0.5*e122*e38+0.5*e152*e38+1.5*e38*e182;
        A[8 + 10*13]=e22*e30*e06-e28*e34*e04+e05*e35*e28+e02*e22*e38+e22*e00*e36+e22*e31*e07+e22*e01*e37+e02*e32*e28+e02*e21*e37-e38*e20*e00-e28*e31*e01-e38*e23*e03-e38*e21*e01-e38*e24*e04-e28*e30*e00-e08*e30*e20-e08*e31*e21-e08*e33*e23-e08*e34*e24-e28*e33*e03+e35*e24*e07+e35*e04*e27+e08*e36*e26+e08*e37*e27+3*e08*e38*e28+e28*e36*e06+e28*e37*e07+e38*e26*e06+e38*e27*e07+e25*e04*e37+e25*e35*e08+e35*e23*e06+e35*e03*e26+e05*e23*e36+e05*e25*e38+e05*e34*e27+e05*e24*e37+e25*e33*e06+e25*e03*e36+e25*e34*e07+e05*e33*e26+e32*e21*e07+e32*e01*e27+e22*e32*e08+e32*e20*e06+e32*e00*e26+e02*e30*e26+e02*e20*e36+e02*e31*e27;
        A[8 + 10*14]=e35*e13*e26-e38*e21*e11-e38*e24*e14+e35*e24*e17+e35*e14*e27+e18*e36*e26+e18*e37*e27+3*e18*e38*e28+e28*e36*e16+e28*e37*e17+e38*e26*e16+e38*e27*e17-e18*e30*e20-e18*e31*e21-e18*e33*e23-e18*e34*e24-e28*e30*e10-e28*e33*e13-e28*e31*e11-e28*e34*e14-e38*e20*e10-e38*e23*e13+e35*e23*e16+e12*e20*e36+e12*e30*e26+e12*e31*e27+e12*e21*e37+e12*e32*e28+e12*e22*e38+e22*e30*e16+e22*e10*e36+e22*e32*e18+e22*e31*e17+e22*e11*e37+e32*e20*e16+e32*e10*e26+e32*e21*e17+e32*e11*e27+e15*e33*e26+e15*e23*e36+e15*e35*e28+e15*e25*e38+e15*e34*e27+e15*e24*e37+e25*e33*e16+e25*e13*e36+e25*e34*e17+e25*e14*e37+e25*e35*e18;
        A[8 + 10*15]=-e28*e30*e20+e22*e30*e26+e22*e20*e36+e22*e31*e27+e22*e21*e37+e22*e32*e28+e32*e20*e26+e32*e21*e27+e25*e33*e26+e25*e23*e36+e25*e35*e28+e25*e34*e27+e25*e24*e37+e35*e23*e26+e35*e24*e27+e28*e36*e26+e28*e37*e27-e28*e31*e21-e28*e33*e23-e28*e34*e24-0.5*e38*e242+0.5*e252*e38+1.5*e38*e282+0.5*e38*e262+0.5*e38*e272-0.5*e38*e202-0.5*e38*e212-0.5*e38*e232+0.5*e222*e38;
        A[8 + 10*16]=-0.5*e08*e312-0.5*e08*e342+0.5*e352*e08+0.5*e08*e362+1.5*e08*e382+0.5*e08*e372-0.5*e08*e302-0.5*e08*e332+e02*e30*e36+e02*e32*e38+e02*e31*e37+e32*e30*e06+e32*e00*e36+e32*e31*e07+e32*e01*e37+e05*e33*e36+e05*e34*e37+e05*e35*e38+e35*e33*e06+e35*e03*e36+e35*e34*e07+e35*e04*e37+0.5*e322*e08+e38*e36*e06+e38*e37*e07-e38*e30*e00-e38*e33*e03-e38*e31*e01-e38*e34*e04;
        A[8 + 10*17]=-e38*e30*e10+e38*e36*e16+e38*e37*e17-e38*e33*e13-e38*e31*e11-e38*e34*e14+0.5*e18*e362+e12*e30*e36+e12*e32*e38+e12*e31*e37+e32*e30*e16+e32*e10*e36+e32*e31*e17+e32*e11*e37+e15*e33*e36+e15*e34*e37+e15*e35*e38+e35*e33*e16+e35*e13*e36+e35*e34*e17+e35*e14*e37+0.5*e322*e18+0.5*e352*e18+1.5*e18*e382+0.5*e18*e372-0.5*e18*e302-0.5*e18*e332-0.5*e18*e312-0.5*e18*e342;
        A[8 + 10*18]=-e38*e30*e20+e25*e35*e38+e22*e30*e36+e22*e32*e38+e22*e31*e37+e32*e30*e26+e32*e20*e36+e32*e31*e27+e32*e21*e37+e25*e33*e36+e25*e34*e37+e35*e33*e26+e35*e23*e36+e35*e34*e27+e35*e24*e37+e38*e36*e26+e38*e37*e27-e38*e31*e21-e38*e33*e23-e38*e34*e24-0.5*e28*e332-0.5*e28*e312-0.5*e28*e342+0.5*e322*e28+0.5*e352*e28+0.5*e28*e362+1.5*e28*e382+0.5*e28*e372-0.5*e28*e302;
        A[8 + 10*19]=e32*e30*e36+0.5*e322*e38+e32*e31*e37+e35*e33*e36+e35*e34*e37+0.5*e352*e38+0.5*e38*e362+0.5*e383+0.5*e38*e372-0.5*e38*e302-0.5*e38*e332-0.5*e38*e312-0.5*e38*e342;
        A[9 + 10*0]=e00*e04*e08-e00*e05*e07+e03*e02*e07-e03*e01*e08-e06*e02*e04+e06*e01*e05;
        A[9 + 10*1]=e06*e01*e15-e16*e02*e04+e16*e01*e05+e03*e02*e17-e13*e01*e08+e06*e11*e05+e13*e02*e07+e00*e04*e18+e00*e14*e08-e00*e05*e17-e10*e05*e07-e00*e15*e07-e06*e12*e04-e06*e02*e14-e03*e01*e18-e03*e11*e08+e10*e04*e08+e03*e12*e07;
        A[9 + 10*2]=-e13*e01*e18-e13*e11*e08+e13*e12*e07+e13*e02*e17+e03*e12*e17-e10*e15*e07+e10*e04*e18+e10*e14*e08-e10*e05*e17-e00*e15*e17+e00*e14*e18+e16*e01*e15+e06*e11*e15-e06*e12*e14-e16*e12*e04-e16*e02*e14+e16*e11*e05-e03*e11*e18;
        A[9 + 10*3]=e10*e14*e18-e10*e15*e17-e13*e11*e18+e13*e12*e17+e16*e11*e15-e16*e12*e14;
        A[9 + 10*4]=-e20*e05*e07+e03*e22*e07+e06*e21*e05+e06*e01*e25-e23*e01*e08+e23*e02*e07+e00*e24*e08-e00*e25*e07-e00*e05*e27+e00*e04*e28-e06*e22*e04-e06*e02*e24-e03*e21*e08-e03*e01*e28-e26*e02*e04+e26*e01*e05+e03*e02*e27+e20*e04*e08;
        A[9 + 10*5]=e23*e12*e07-e26*e02*e14+e16*e21*e05-e23*e11*e08+e10*e24*e08-e20*e05*e17+e26*e11*e05+e26*e01*e15+e10*e04*e28+e00*e24*e18-e00*e15*e27+e03*e22*e17-e13*e01*e28+e23*e02*e17+e16*e01*e25+e20*e04*e18+e06*e11*e25+e13*e02*e27-e23*e01*e18-e20*e15*e07-e10*e25*e07+e13*e22*e07-e06*e22*e14-e26*e12*e04-e03*e11*e28-e03*e21*e18-e16*e22*e04-e16*e02*e24-e06*e12*e24+e06*e21*e15+e00*e14*e28-e00*e25*e17+e20*e14*e08-e13*e21*e08-e10*e05*e27+e03*e12*e27;
        A[9 + 10*6]=-e13*e11*e28+e13*e12*e27+e13*e22*e17+e16*e11*e25+e10*e14*e28-e13*e21*e18-e23*e11*e18+e23*e12*e17+e20*e14*e18-e20*e15*e17+e26*e11*e15-e10*e15*e27-e10*e25*e17-e16*e22*e14-e16*e12*e24+e16*e21*e15-e26*e12*e14+e10*e24*e18;
        A[9 + 10*7]=e26*e21*e05+e26*e01*e25+e20*e04*e28+e20*e24*e08-e20*e25*e07+e23*e22*e07+e03*e22*e27-e03*e21*e28-e26*e22*e04-e20*e05*e27-e00*e25*e27+e06*e21*e25-e06*e22*e24+e00*e24*e28-e26*e02*e24-e23*e21*e08-e23*e01*e28+e23*e02*e27;
        A[9 + 10*8]=-e10*e25*e27+e10*e24*e28-e20*e15*e27-e20*e25*e17+e20*e14*e28+e20*e24*e18+e26*e11*e25+e23*e22*e17-e23*e11*e28+e23*e12*e27-e23*e21*e18-e13*e21*e28+e13*e22*e27-e26*e12*e24+e26*e21*e15-e16*e22*e24+e16*e21*e25-e26*e22*e14;
        A[9 + 10*9]=-e20*e25*e27+e20*e24*e28-e23*e21*e28+e23*e22*e27-e26*e22*e24+e26*e21*e25;
        A[9 + 10*10]=e03*e02*e37-e03*e31*e08-e03*e01*e38+e03*e32*e07-e00*e35*e07+e30*e04*e08+e06*e31*e05-e36*e02*e04+e36*e01*e05-e06*e32*e04-e06*e02*e34+e06*e01*e35+e00*e04*e38-e00*e05*e37+e33*e02*e07-e33*e01*e08-e30*e05*e07+e00*e34*e08;
        A[9 + 10*11]=-e36*e12*e04+e30*e04*e18-e30*e15*e07-e36*e02*e14-e30*e05*e17+e30*e14*e08-e00*e35*e17-e00*e15*e37+e33*e02*e17-e06*e32*e14-e06*e12*e34-e16*e32*e04+e06*e31*e15+e06*e11*e35+e00*e34*e18-e10*e35*e07-e33*e11*e08-e33*e01*e18+e16*e01*e35-e16*e02*e34+e16*e31*e05-e03*e31*e18-e03*e11*e38+e03*e32*e17+e13*e02*e37-e13*e31*e08-e13*e01*e38+e10*e34*e08+e00*e14*e38+e36*e11*e05+e36*e01*e15+e03*e12*e37-e10*e05*e37+e10*e04*e38+e33*e12*e07+e13*e32*e07;
        A[9 + 10*12]=-e36*e12*e14-e30*e15*e17+e13*e32*e17-e13*e31*e18-e33*e11*e18+e33*e12*e17+e10*e14*e38+e30*e14*e18-e13*e11*e38+e13*e12*e37-e10*e35*e17+e10*e34*e18-e16*e12*e34-e16*e32*e14+e16*e11*e35+e16*e31*e15+e36*e11*e15-e10*e15*e37;
        A[9 + 10*13]=-e06*e22*e34-e06*e32*e24-e00*e25*e37-e00*e35*e27+e23*e02*e37+e00*e24*e38-e23*e01*e38-e03*e31*e28-e33*e01*e28+e03*e22*e37+e03*e32*e27+e33*e02*e27-e03*e21*e38-e26*e32*e04-e33*e21*e08+e36*e01*e25+e36*e21*e05-e20*e05*e37+e20*e04*e38+e30*e04*e28-e20*e35*e07+e33*e22*e07+e30*e24*e08-e30*e25*e07-e23*e31*e08+e23*e32*e07+e00*e34*e28+e06*e21*e35+e06*e31*e25-e36*e02*e24+e26*e01*e35-e36*e22*e04+e26*e31*e05-e26*e02*e34+e20*e34*e08-e30*e05*e27;
        A[9 + 10*14]=e33*e22*e17+e33*e12*e27+e16*e21*e35-e16*e22*e34-e16*e32*e24+e23*e32*e17-e23*e11*e38-e23*e31*e18+e23*e12*e37-e13*e21*e38-e13*e31*e28+e13*e22*e37+e36*e21*e15-e36*e12*e24+e36*e11*e25-e26*e12*e34-e20*e35*e17+e20*e14*e38+e20*e34*e18+e30*e24*e18-e30*e15*e27-e30*e25*e17+e30*e14*e28-e33*e21*e18+e10*e34*e28+e10*e24*e38-e10*e35*e27-e10*e25*e37-e20*e15*e37-e26*e32*e14+e26*e11*e35+e26*e31*e15-e36*e22*e14+e13*e32*e27+e16*e31*e25-e33*e11*e28;
        A[9 + 10*15]=-e20*e35*e27-e20*e25*e37+e20*e34*e28+e20*e24*e38+e30*e24*e28-e30*e25*e27+e23*e32*e27+e23*e22*e37-e23*e31*e28-e23*e21*e38+e33*e22*e27-e26*e22*e34-e26*e32*e24+e26*e21*e35+e26*e31*e25-e36*e22*e24+e36*e21*e25-e33*e21*e28;
        A[9 + 10*16]=-e33*e01*e38-e03*e31*e38+e00*e34*e38+e33*e32*e07+e03*e32*e37+e06*e31*e35-e00*e35*e37-e36*e32*e04-e06*e32*e34-e36*e02*e34+e36*e01*e35+e36*e31*e05+e30*e04*e38+e30*e34*e08-e33*e31*e08+e33*e02*e37-e30*e05*e37-e30*e35*e07;
        A[9 + 10*17]=-e33*e31*e18-e33*e11*e38+e10*e34*e38+e30*e14*e38-e10*e35*e37-e30*e15*e37-e13*e31*e38+e13*e32*e37-e30*e35*e17+e33*e12*e37+e30*e34*e18+e33*e32*e17+e16*e31*e35-e16*e32*e34-e36*e12*e34-e36*e32*e14+e36*e11*e35+e36*e31*e15;
        A[9 + 10*18]=-e20*e35*e37+e20*e34*e38+e30*e24*e38-e30*e35*e27-e30*e25*e37+e30*e34*e28+e23*e32*e37-e23*e31*e38-e33*e21*e38-e33*e31*e28+e33*e22*e37+e33*e32*e27+e26*e31*e35-e26*e32*e34-e36*e22*e34-e36*e32*e24+e36*e21*e35+e36*e31*e25;
        A[9 + 10*19]=-e33*e31*e38-e30*e35*e37+e36*e31*e35+e33*e32*e37+e30*e34*e38-e36*e32*e34;
    }

    // Transform vector A to matrix.
    for ( int i = 0; i < 10; i++ )
    {
        for ( int j = 0; j < 20; j++ )
        {
            cvmSet( AMat, i, j, A[j*10 + i] );
        }
    }
}

void Odometry::estimateTranslation( CvMat *R, CvMat *T, vector<CvScalar> last, vector<CvScalar> next, double &scale )
{

}

void Odometry::getCameraMatrices( CvMat *E, vector<CvMat *> &Rs, vector<CvMat *> &Ts )
{
    CvMat *EET = cvCreateMat( 3, 3, CV_32F );
    cvGEMM( E, E, 1, NULL, 1, EET, CV_GEMM_B_T );

    // Obtain bxbT
    CvMat *bbt = cvCreateMat( 3, 3, CV_32F );
    CvMat *I = cvCreateMat( 3, 3, CV_32F );
    cvSetIdentity( I, cvScalar(1) );
    cvGEMM( I, NULL, 0.5 * cvTrace( EET ).val[0], EET, -1, bbt );

    // We obtain b by selecting the largest row and dividing by the square
    // root of the diagonal. This if for numerical accuracy.
    CvMat *b1 = cvCreateMat( 3, 1, CV_32F );
    CvMat *b2 = cvCreateMat( 3, 1, CV_32F );
    int j = 0;
//    if ( ( cvmGet( bbt, 0, 0 ) >= cvmGet( bbt, 1, 1 ) ) &&
//         ( cvmGet( bbt, 0, 0 ) >= cvmGet( bbt, 2, 2 ) ) )
//    {
//        j = 0;
//    }
    if ( ( cvmGet( bbt, 1, 1 ) >= cvmGet( bbt, 0, 0 ) ) &&
         ( cvmGet( bbt, 1, 1 ) >= cvmGet( bbt, 3, 3 ) ) )
    {
        j = 1;
    }
    if ( ( cvmGet( bbt, 2, 2 ) >= cvmGet( bbt, 0, 0 ) ) &&
         ( cvmGet( bbt, 2, 2 ) >= cvmGet( bbt, 1, 1 ) ) )
    {
        j = 2;
    }

    float sq = sqrt( (float)cvmGet( bbt, j, j ) );
    for ( int i = 0; i < 3; i++ )
    {
        cvmSet( b1, i, 0, cvmGet( bbt, j, i ) / sq );
        cvmSet( b2, i, 0, -cvmGet( b1, i, 0 ) );
    }

    // Compute cofactors of E
    CvMat *E1 = cvCreateMat( 3, 1, CV_32F );
    CvMat *E2 = cvCreateMat( 3, 1, CV_32F );
    CvMat *E3 = cvCreateMat( 3, 1, CV_32F );
    CvMat *E23 = cvCreateMat( 3, 1, CV_32F );
    CvMat *E31 = cvCreateMat( 3, 1, CV_32F );
    CvMat *E12 = cvCreateMat( 3, 1, CV_32F );
    CvMat *cofE = cvCreateMat( 3, 3, CV_32F );
    for ( int i = 0; i < 3; i++ )
    {
        cvmSet( E1, i, 0, cvmGet( E, i, 0 ) );
        cvmSet( E2, i, 0, cvmGet( E, i, 1 ) );
        cvmSet( E3, i, 0, cvmGet( E, i, 2 ) );
    }
    cvCrossProduct( E2, E3, E23 );
    cvCrossProduct( E3, E1, E31 );
    cvCrossProduct( E1, E2, E12 );
    for ( int i = 0; i < 3; i++ )
    {
        cvmSet( cofE, 0, i, cvmGet( E23, i, 0 ) );
        cvmSet( cofE, 1, i, cvmGet( E31, i, 0 ) );
        cvmSet( cofE, 2, i, cvmGet( E12, i, 0 ) );
    }

    // Compute two matricex B.
    CvMat *B1 = cvCreateMat( 3, 3, CV_32F );
    CvMat *B2 = cvCreateMat( 3, 3, CV_32F );
    cvmSet( B1, 0, 1, -cvmGet( b1, 2, 0 ) );
    cvmSet( B1, 0, 2,  cvmGet( b1, 1, 0 ) );
    cvmSet( B1, 1, 0,  cvmGet( b1, 2, 0 ) );
    cvmSet( B1, 1, 2, -cvmGet( b1, 0, 0 ) );
    cvmSet( B1, 2, 0, -cvmGet( b1, 1, 0 ) );
    cvmSet( B1, 2, 1,  cvmGet( b1, 0, 0 ) );
    cvmSet( B2, 0, 1, -cvmGet( b2, 2, 0 ) );
    cvmSet( B2, 0, 2,  cvmGet( b2, 1, 0 ) );
    cvmSet( B2, 1, 0,  cvmGet( b2, 2, 0 ) );
    cvmSet( B2, 1, 2, -cvmGet( b2, 0, 0 ) );
    cvmSet( B2, 2, 0, -cvmGet( b2, 1, 0 ) );
    cvmSet( B2, 2, 1,  cvmGet( b2, 0, 0 ) );

    double b1dot = cvDotProduct( b1, b1 );
    double b2dot = cvDotProduct( b2, b2 );

    // Compute both R
    CvMat *R1 = cvCreateMat( 3, 3, CV_32F );
    CvMat *R2 = cvCreateMat( 3, 3, CV_32F );
    cvGEMM( B1, E, -1/b1dot, cofE, 1/b1dot, R1, CV_GEMM_C_T );
    cvGEMM( B2, E, -1/b2dot, cofE, 1/b2dot, R2, CV_GEMM_C_T );

    // Build 4 possible solutions
    Rs.push_back( R1 );
    Rs.push_back( R2 );
    Ts.push_back( b1 );
    Ts.push_back( b2 );

    cvReleaseMat( &EET );
    cvReleaseMat( &I );
    cvReleaseMat( &bbt );
    cvReleaseMat( &E1 );
    cvReleaseMat( &E2 );
    cvReleaseMat( &E3 );
    cvReleaseMat( &E23 );
    cvReleaseMat( &E31 );
    cvReleaseMat( &E12 );
    cvReleaseMat( &cofE );
    cvReleaseMat( &B1 );
    cvReleaseMat( &B2 );
}

void Odometry::getCorrectCameraMatrix( vector<CvMat *> &Rs, vector<CvMat *> &Ts, CvMat *Rcorrect, CvMat *Tcorrect,
                                       CvScalar x1, CvScalar x2 )
{
    // For each camera matrix (Pxcam), reproject the pair of points in 3D
    // and determine the depth in 3D of the point
    // FIRST I DO IT FOR ONE
    CvMat *X = cvCreateMat( 3, 1, CV_32F );
    CvMat *A1 = cvCreateMat( 1, 4, CV_32F );
    cvSet( A1, cvScalar( 0 ) );
    CvMat *A2 = cvCreateMat( 1, 4, CV_32F );
    cvSet( A2, cvScalar( 0 ) );
    CvMat *A3 = cvCreateMat( 1, 4, CV_32F );
    CvMat *A4 = cvCreateMat( 1, 4, CV_32F );
    CvMat *A = cvCreateMat( 4, 4, CV_32F );
    CvMat *U = cvCreateMat( 4, 4, CV_32F );
    CvMat *S = cvCreateMat( 4, 4, CV_32F );
    CvMat *V = cvCreateMat( 4, 4, CV_32F );
    for ( int i = 0; i < 3; i++ )
    {
        CvMat *R;
        CvMat *T;
        switch ( i )
        {
        case 0:
            R = Rs[0];
            T = Ts[0];
            break;
        case 1:
            R = Rs[0];
            T = Ts[1];
            break;
        case 2:
            R = Rs[1];
            T = Ts[0];
            break;
        case 3:
            R = Rs[1];
            T = Ts[1];
            break;
        }

        cvmSet( A1, 0, 0, -1 );
        cvmSet( A1, 0, 2, x1.val[0] );
        cvmSet( A2, 0, 1, -1 );
        cvmSet( A2, 0, 2, x1.val[1] );

        for ( int j = 0; j < 3; j++ )
        {
            cvmSet( A3, 0, j, cvmGet( R, 2, j )*x2.val[0] - cvmGet( R, 0, j ) );
            cvmSet( A4, 0, j, cvmGet( R, 2, j )*x2.val[1] - cvmGet( R, 1, j ) );
        }
        cvmSet( A3, 0, 3, cvmGet( T, 2, 0 )*x2.val[0] - cvmGet( T, 0, 0 ) );
        cvmSet( A4, 0, 3, cvmGet( T, 2, 0 )*x2.val[1] - cvmGet( T, 1, 0 ) );

        // Normalize A.
        double normA1 = cvNorm( A1 );
        double normA2 = cvNorm( A2 );
        double normA3 = cvNorm( A3 );
        double normA4 = cvNorm( A4 );
        for ( int j = 0; j < 4; j++ )
        {
            cvmSet( A, 0, j, cvmGet( A1, 0, j ) / normA1 );
            cvmSet( A, 1, j, cvmGet( A2, 0, j ) / normA2 );
            cvmSet( A, 2, j, cvmGet( A3, 0, j ) / normA3 );
            cvmSet( A, 3, j, cvmGet( A4, 0, j ) / normA4 );
        }

        // Obtain the 3D point.
        cvSVD( A, S, U, V );
        for ( int j = 0; j < 3; j++ )
        {
            cvmSet( X, j, 0, cvmGet( V, j, 0 ) );
        }
        double X4 = cvmGet( V, 3, 0 );

        // Check depth on first camera.
        double w = cvmGet( X, 2, 0 );

        if ( w / X4 > 0 )
        {
            // Check depth on second camera
            CvMat *xi = cvCreateMat( 3, 1, CV_32F );
            cvGEMM( R, X, 1.0, T, X4, xi );
            w = cvmGet( xi, 2, 0 );
            cvReleaseMat( &xi );
            double m3n = 0.0;
            for ( int j = 0; j < 3; j++ )
            {
                m3n += cvmGet( R, 2, j ) * cvmGet( R, 2, j );
            }
            m3n = sqrt( m3n );
            if ( ( cvDet( R ) * w ) / ( X4 * m3n ) > 0 )
            {
                cvCopy( R, Rcorrect );
                cvCopy( T, Tcorrect );

                cvReleaseMat( &A1 );
                cvReleaseMat( &A2 );
                cvReleaseMat( &A3 );
                cvReleaseMat( &A4 );
                cvReleaseMat( &A );
                cvReleaseMat( &S );
                cvReleaseMat( &U );
                cvReleaseMat( &V );
                cvReleaseMat( &X );
                return;
            }
        }
    }

    cvCopy( Rs[3], Rcorrect );
    cvCopy( Ts[3], Tcorrect );

    cvReleaseMat( &A1 );
    cvReleaseMat( &A2 );
    cvReleaseMat( &A3 );
    cvReleaseMat( &A4 );
    cvReleaseMat( &A );
    cvReleaseMat( &S );
    cvReleaseMat( &U );
    cvReleaseMat( &V );
    cvReleaseMat( &X );
}

void Odometry::drawRandomSamples( int numSamples, int numTotal, bool *selected )
{
    while (numSamples != 0)
    {
        int r = rand() % numTotal
                ;
//        qDebug( "%d", r );
        if ( !selected[r] )
        {
            selected[r] = true;
            numSamples--;
        }
    }
}