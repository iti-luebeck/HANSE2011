#include "waypoint.h"

Waypoint::Waypoint()
{
    this->name = "BLA";
    this->posX = 0;
    this->posY = 0;
    this->depth = 0;
    this->useExitAngle = false;
    this->startAngle = 0;
    this->useExitAngle = false;
    this->exitAngle = 0;
}

Waypoint::Waypoint(QString name, double posX, double posY, double depth, bool useStartAngle, double startAngle, bool useExitAngle, double exitAngle)
{
    this->name = name;
    this->posX = posX;
    this->posY = posY;
    this->depth = depth;
    this->useStartAngle = useStartAngle;
    this->startAngle = startAngle;
    this->useExitAngle = useExitAngle;
    this->exitAngle = exitAngle;
}

Waypoint::Waypoint(const Waypoint &copy)
{
    this->name = copy.name;
    this->posX = copy.posX;
    this->posY = copy.posY;
    this->depth = copy.depth;
    this->useStartAngle = copy.useStartAngle;
    this->startAngle = copy.startAngle;
    this->useExitAngle = copy.useExitAngle;
    this->exitAngle = copy.exitAngle;
}

Waypoint::~Waypoint()
{

}

//Waypoint& Waypoint::operator=( const Waypoint& copy)
//{
//    this->name = copy.name;
//    this->posX = copy.posX;
//    this->posY = copy.posY;
//    this->depth = copy.depth;
//    this->useStartAngle = copy.useStartAngle;
//    this->startAngle = copy.startAngle;
//    this->useExitAngle = copy.useExitAngle;
//    this->exitAngle = copy.exitAngle;
//    return *this;
//}

QDataStream &operator<<(QDataStream &out, const Waypoint &obj)
{
    out << obj.name << obj.posX << obj.posY << obj.depth << obj.useStartAngle << obj.startAngle << obj.useExitAngle << obj.exitAngle;
    return out;
}

QDataStream &operator>>(QDataStream &in, Waypoint &obj)
{
    in >> obj.name >> obj.posX >> obj.posY >> obj.depth >> obj.useStartAngle >> obj.startAngle >> obj.useExitAngle >> obj.exitAngle;
    return in;
}
