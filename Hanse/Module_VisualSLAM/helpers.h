#ifndef HELPERS_H
#define HELPERS_H

#include <opencv/cxcore.h>
#include <QString>

inline void printMatrix( CvMat *M, const char *name = "Matrix" )
{
    qDebug( name );
    qDebug( "h = %d, w = %d", M->height, M->width );
    QString str;
    for ( int i = 0; i < M->rows; i++ )
    {
        for ( int j = 0; j < M->cols; j++ )
        {
            str.append("%1 ");
            str = str.arg( cvmGet( M, i, j ), 0, 'f', 4 );
        }
        str.append("\n");
    }
    qDebug( str.toStdString().c_str() );
}

inline double mvnpdf( CvMat *x, CvMat *m, CvMat *S )
{
    // Subtract mean from data point.
    if ( m != NULL )
    {
        cvSub( x, m, x );
    }

    double k = S->rows;
    double detS = cvDet( S );
    CvMat *Sinv = cvCreateMat( S->rows, S->cols, S->type );
    cvInvert( S, Sinv, CV_SVD );
    CvMat *xtemp = cvCreateMat( x->rows, x->cols, x->type );
    cvGEMM( Sinv, x, -0.5, NULL, 1, xtemp );
    double e = exp( cvDotProduct( x, xtemp ) ) / ( pow( 2 * CV_PI, k / 2 ) * sqrt( detS ) );
    cvReleaseMat( &Sinv );
    cvReleaseMat( &xtemp );
    return e;
}

inline void cholesky( CvMat *A, CvMat *L )
{
    double Ltemp;
    cvSetIdentity( L, cvScalar( 1 ) );
    CvMat *D = cvCreateMat( L->rows, L->cols, L->type );
    cvSetZero( D );

    cvmSet( D, 0, 0, sqrt( cvmGet( A, 0, 0 ) ) );
    for ( int j = 0; j < A->rows; j++ )
    {
        if ( j > 0 )
        {
            Ltemp = 0;
            for ( int k = 0; k < j-1; k++ )
            {
                Ltemp = Ltemp + cvmGet( L, k, j ) * cvmGet( L, k, j ) * cvmGet( D, k, k );
            }
            cvmSet( D, j, j,  cvmGet( A, j, j ) - Ltemp );
        }
        for ( int i = j + 1; i < A->cols; i++ )
        {
            Ltemp = 0;
            for ( int k = 0; k < j-1; k++ )
            {
                Ltemp = Ltemp + cvmGet( L, k, i ) * cvmGet( L, k, j ) * cvmGet( D, k, k );
            }
            cvmSet( L, j, i, ( cvmGet( A, j, i ) - Ltemp ) / cvmGet( D, j, j ) );
        }
    }

    for ( int i = 0; i < D->rows; i++ )
    {
        if ( cvmGet( D, i, i ) > 0 )
        {
            cvmSet( D, i, i, sqrt( cvmGet( D, i, i ) ) );
        }
        else
        {
            cvmSet( D, i, i, 0 );
        }
    }

    cvGEMM( D, L, 1, NULL, 1, L );

    cvReleaseMat( &D );
}

#endif // HELPERS_H
