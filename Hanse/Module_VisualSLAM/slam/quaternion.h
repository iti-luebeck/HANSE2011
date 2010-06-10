#ifndef QUATERNION_H
#define QUATERNION_H

#include <opencv/cxcore.h>
#include <QString>

class Quaternion
{
public:
    Quaternion( float w = 1, float x = 0, float y = 0, float z = 0 );
    float w;
    float x;
    float y;
    float z;
    CvMat *getRotation();
    void getYawPitchRoll( float &yaw, float &pitch, float &roll );
    static Quaternion fromYawPitchRoll( float yaw, float pitch, float roll );
    static Quaternion fromRotation( CvMat *R );
    void rotate( Quaternion q );
    static Quaternion rotate( Quaternion q1, Quaternion q2 );
    static void rotate( Quaternion q, CvMat *x );
    float norm();
    void normalize();
    Quaternion operator+( const Quaternion q );
    Quaternion operator-( const Quaternion q );
    Quaternion operator*( const Quaternion q );
    QString toString();
};

#endif // QUATERNION_H
