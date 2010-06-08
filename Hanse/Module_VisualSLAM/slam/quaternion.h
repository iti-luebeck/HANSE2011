#ifndef QUATERNION_H
#define QUATERNION_H

#include <opencv/cxcore.h>
#include <QString>

class Quaternion
{
public:
    Quaternion( double w = 1, double x = 0, double y = 0, double z = 0 );
    double w;
    double x;
    double y;
    double z;
    CvMat *getRotation();
    void getYawPitchRoll( double &yaw, double &pitch, double &roll );
    static Quaternion fromYawPitchRoll( double yaw, double pitch, double roll );
    static Quaternion fromRotation( CvMat *R );
    void rotate( Quaternion q );
    static Quaternion rotate( Quaternion q1, Quaternion q2 );
    static void rotate( Quaternion q, CvMat *x );
    double norm();
    void normalize();
    Quaternion operator+( const Quaternion q );
    Quaternion operator-( const Quaternion q );
    Quaternion operator*( const Quaternion q );
    QString toString();
};

#endif // QUATERNION_H
