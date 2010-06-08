#include "quaternion.h"

Quaternion::Quaternion( double w, double x, double y, double z )
{
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
}

CvMat *Quaternion::getRotation()
{
    CvMat *R = cvCreateMat( 3, 3, CV_32F );

    cvmSet( R, 0, 0, 1-2*(y*y + z*z) );
    cvmSet( R, 0, 1,   2*(x*y - w*z) );
    cvmSet( R, 0, 2,   2*(x*z + w*y) );

    cvmSet( R, 1, 0,   2*(x*y + w*z) );
    cvmSet( R, 1, 1, 1-2*(z*z + x*x) );
    cvmSet( R, 1, 2,   2*(y*z - w*x) );

    cvmSet( R, 2, 0,   2*(x*z - w*y) );
    cvmSet( R, 2, 1,   2*(y*z + w*x) );
    cvmSet( R, 2, 2, 1-2*(x*x + y*y) );

    return R;
}

void Quaternion::getYawPitchRoll( double &yaw, double &pitch, double &roll )
{
    yaw = atan2( 2 * ( x*y + w*z ), w*w  + x*x - y*y - z*z ) * 180 / CV_PI;
    pitch = asin( -2 * ( x*z - w*y ) ) * 180 / CV_PI;
    roll = atan2( 2 * ( w*x + y*z ), w*w - x*x - y*y + z*z ) * 180 / CV_PI;

    double temp = pitch;
    pitch = roll;
    roll = yaw;
    yaw = temp;
}

Quaternion Quaternion::fromRotation( CvMat *R )
{
    Quaternion q;

    double d0 = cvmGet( R, 0, 0 );
    double d1 = cvmGet( R, 1, 1 );
    double d2 = cvmGet( R, 2, 2 );
    double xx = 1 + d0 - d1 - d2;
    double yy = 1 - d0 + d1 - d2;
    double zz = 1 - d0 - d1 + d2;
    double ww = 1 + d0 + d1 + d2;

    if ( ww >= xx && ww >= yy && ww >= zz )
    {
        double ws = 2 * sqrt(ww);
        q.x = ( cvmGet( R, 2, 1 ) - cvmGet( R, 1, 2 ) ) / ws;
        q.y = ( cvmGet( R, 0, 2 ) - cvmGet( R, 2, 0 ) ) / ws;
        q.z = ( cvmGet( R, 1, 0 ) - cvmGet( R, 0, 1 ) ) / ws;
        q.w = ws / 4;
    }
    else if ( xx >= yy && xx >= zz )
    {
        double xs = 2 * sqrt(xx);
        q.x = xs / 4;
        q.y = ( cvmGet( R, 1, 0 ) + cvmGet( R, 0, 1 ) ) / xs;
        q.z = ( cvmGet( R, 2, 0 ) + cvmGet( R, 0, 2 ) ) / xs;
        q.w = ( cvmGet( R, 2, 1 ) - cvmGet( R, 1, 2 ) ) / xs;
    }
    else if ( yy >= zz )
    {
        double ys = 2 * sqrt(yy);
        q.x = ( cvmGet( R, 1, 0 ) + cvmGet( R, 0, 1 ) ) / ys;
        q.y = ys / 4;
        q.z = ( cvmGet( R, 2, 1 ) + cvmGet( R, 1, 2 ) ) / ys;
        q.w = ( cvmGet( R, 0, 2 ) - cvmGet( R, 2, 0 ) ) / ys;
    }
    else
    {
        double zs = 2 * sqrt(zz);
        q.x = ( cvmGet( R, 2, 0 ) + cvmGet( R, 0, 2 ) ) / zs;
        q.y = ( cvmGet( R, 2, 1 ) + cvmGet( R, 1, 2 ) ) / zs;
        q.z = zs / 4;
        q.w = ( cvmGet( R, 1, 0 ) - cvmGet( R, 0, 1 ) ) / zs;
    }

    return q;
}

Quaternion Quaternion::fromYawPitchRoll( double yaw, double pitch, double roll )
{
    double cy = cos( yaw / 2 );
    double cp = cos( pitch / 2 );
    double cr = cos( roll / 2 );
    double sy = sin( yaw / 2 );
    double sp = sin( pitch / 2 );
    double sr = sin( roll / 2 );

    Quaternion q;
    q.w = cy * cp * cr + sy * sp * sr;
    q.x = cy * cp * sr - sy * sp * cr;
    q.y = cy * sp * cr + sy * cp * sr;
    q.z = sy * cp * cr - cy * sp * sr;

    return q;
}

void Quaternion::rotate( Quaternion q )
{
    Quaternion qr = rotate( Quaternion( w, x, y, z ), q );
    w = qr.w;
    x = qr.x;
    y = qr.y;
    z = qr.z;
}

void Quaternion::rotate( Quaternion q, CvMat *v )
{
    Quaternion qv( 0, cvmGet( v, 0, 0 ), cvmGet( v, 1, 0 ), cvmGet( v, 2, 0 ) );
    Quaternion qconj( q.w, -q.x, -q.y, -q.z );
    Quaternion qxrot = q * qv * qconj;
    cvmSet( v, 0, 0, qxrot.x );
    cvmSet( v, 1, 0, qxrot.y );
    cvmSet( v, 2, 0, qxrot.z );
}

Quaternion Quaternion::rotate( Quaternion q1, Quaternion q2 )
{
    return q1 * q2;
}

double Quaternion::norm()
{

    return sqrt( w*w + x*x + y*y + z*z );
}

void Quaternion::normalize()
{
    double d = norm();
    w = w / d;
    x = x / d;
    y = y / d;
    z = z / d;
}

Quaternion Quaternion::operator+( const Quaternion q )
{
    return Quaternion( w + q.w, x + q.x, y + q.y, z + q.z );
}

Quaternion Quaternion::operator-( const Quaternion q )
{
    return Quaternion( w - q.w, x - q.x, y - q.y, z - q.z );
}

Quaternion Quaternion::operator*( const Quaternion q )
{
    return Quaternion( w*q.w - x*q.x - y*q.y - z*q.z,
                       w*q.x + x*q.w + y*q.z - z*q.y,
                       w*q.y - x*q.z + y*q.w + z*q.x,
                       w*q.z + x*q.y - y*q.x + z*q.w );
}

QString Quaternion::toString()
{
    return QString( "(%1,%2,%3,%4)" ).arg( w, 0, 'f', 3 )
            .arg( x, 0, 'f', 3 )
            .arg( y, 0, 'f', 3 )
            .arg( z, 0, 'f', 3 );
}
