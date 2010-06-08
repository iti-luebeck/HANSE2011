#ifndef POSITION_H
#define POSITION_H

#include <QObject>

/**
  * Position and orientation.
  *
  * see http://en.wikipedia.org/wiki/Aircraft_principal_axes
  * for an illustration of the coordinate system
  *
  * x to the front of the robot (forward direction; roll axis)
  * y to the right of the robot (sideways direction; pitch axis)
  * z to the bottom of the robot (down direction; yaw axis)
  *
  * (right-handed coordinate system)
  *
  * Units are in meters (m)
  *
  * yaw: rotation along z axis
  * pitch: rotation along y axis
  * roll: rotation along x axis
  *
  * TODO: should this class be used in math code? as a matrix? a vector?
  *
  */
class Position : public QObject
{
public:
    Position( const Position& );
    Position( double x = 0, double y = 0, double z = 0,
              double roll = 0, double pitch = 0, double yaw = 0 );

    Position& operator=( const Position& );

    double getX();
    double getY();
    double getZ();

    double getRoll();
    double getPitch();
    double getYaw();

    void setX(double);
    void setY(double);
    void setZ(double);

    void setYaw(double);
    void setPitch(double);
    void setRoll(double);

private:
    double x;
    double y;
    double z;
    double yaw;
    double pitch;
    double roll;

};

#endif // POSITION_H
