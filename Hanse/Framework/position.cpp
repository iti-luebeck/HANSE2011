#include "position.h"

Position::Position( const Position &pos )
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

Position& Position::operator=( const Position& pos )
{
    setX( pos.x );
    setY( pos.y );
    setZ( pos.z );
    setRoll( pos.roll );
    setPitch( pos.pitch );
    setYaw( pos.yaw );
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
