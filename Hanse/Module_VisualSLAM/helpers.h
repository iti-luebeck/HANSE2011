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

#endif // HELPERS_H
