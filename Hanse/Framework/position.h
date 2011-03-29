#ifndef POSITION_H
#define POSITION_H

#include <QtCore>
#include <QVector3D>
#include <opencv/cxcore.h>
//#include <Module_VisualSLAM/slam/quaternion.h>

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
  * Angles are in deg!
  *
  */
class Position : public QObject
{
public:
    Position( const Position& );
    Position( const QVector3D& pos );
    Position( double x = 0, double y = 0, double z = 0,
              double roll = 0, double pitch = 0, double yaw = 0 );
//    Position( CvMat *translation, Quaternion rotation );

    Position& operator=( const Position& );
    Position operator+( Position pos );
    Position operator-( Position pos );

    // Setters & Getters for using Position as a 6D position.
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

    // Setters & Getters for using Position as a waypoint position.
    double getDepth();
    double getArrivalAngle();
    double getExitAngle();

    void setDepth( double depth );
    void setArrivalAngle( double arrivalAngle );
    void setExitAngle( double exitAngle );

private:
    double x;
    double y;
    double z;
    double yaw;
    double pitch;
    double roll;

};

#endif // POSITION_H
