#include "position.h"

Position::Position( const Position &pos ) : QObject()
{
    setX( pos.x );
    setY( pos.y );
    setZ( pos.z );
    setRoll( pos.roll );
    setPitch( pos.pitch );
    setYaw( pos.yaw );
}

Position::Position( double x, double y, double z, double roll, double pitch, double yaw )
{
    setX( x );
    setY( y );
    setZ( z );
    setRoll( roll );
    setPitch( pitch );
    setYaw( yaw );
}

Position::Position( const QVector3D& pos )
{
    setX( pos.x() );
    setY( pos.y() );
    setYaw( pos.z() );
}


//Position::Position( CvMat *translation, Quaternion rotation )
//{
//    float yaw;
//    float pitch;
//    float roll;
//    rotation.getYawPitchRoll( yaw, pitch, roll );
//    setRoll( (double)roll );
//    setPitch( (double)pitch );
//    setYaw( (double)yaw );
//    setX( -cvmGet( translation, 0, 0 ) );
//    setY( cvmGet( translation, 2, 0 ) );
//    setZ( -cvmGet( translation, 1, 0 ) );
//}

Position& Position::operator=( const Position& pos )
{
    setX( pos.x );
    setY( pos.y );
    setZ( pos.z );
    setRoll( pos.roll );
    setPitch( pos.pitch );
    setYaw( pos.yaw );
    return *this;
}

Position Position::operator+( Position pos )
{
    return Position( getX() + pos.getX(),
                     getY() + pos.getY(),
                     getZ() + pos.getZ(),
                     getRoll() + pos.getRoll(),
                     getPitch() + pos.getPitch(),
                     getYaw() + pos.getYaw() );
}

Position Position::operator-( Position pos )
{
    return Position( getX() - pos.getX(),
                     getY() - pos.getY(),
                     getZ() - pos.getZ(),
                     getRoll() - pos.getRoll(),
                     getPitch() - pos.getPitch(),
                     getYaw() - pos.getYaw() );
}

double Position::getX()
{
    return x;
}

double Position::getY()
{
    return y;
}

double Position::getZ()
{
    return z;
}

double Position::getRoll()
{
    return roll;
}

double Position::getPitch()
{
    return pitch;
}

double Position::getYaw()
{
    return yaw;
}

void Position::setX( double x )
{
    this->x = x;
}

void Position::setY( double y )
{
    this->y = y;
}

void Position::setZ( double z )
{
    this->z = z;
}

void Position::setYaw( double yaw )
{
    this->yaw = yaw;
}

void Position::setPitch( double pitch )
{
    this->pitch = pitch;
}

void Position::setRoll( double roll )
{
    this->roll = roll;
}

double Position::getDepth()
{
    return z;
}

double Position::getArrivalAngle()
{
    return yaw;
}

double Position::getExitAngle()
{
    return pitch;
}

void Position::setDepth( double depth )
{
    z = depth;
}

void Position::setArrivalAngle( double arrivalAngle )
{
    yaw = arrivalAngle;
}

void Position::setExitAngle( double exitAngle )
{
    pitch = exitAngle;
}
