#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QtCore>

class Waypoint// : public QObject
{
public:
    Waypoint();
    Waypoint(QString name, double posX, double posY, double depth, bool useStartAngle, double startAngle, bool useExitAngle, double exitAngle);
    Waypoint(const Waypoint &);
    ~Waypoint();

//    Waypoint& operator=( const Waypoint& );
    friend QDataStream &operator<<(QDataStream &out, const Waypoint &obj);
    friend QDataStream &operator>>(QDataStream &in, Waypoint &obj);

public:
    QString name;

    double posX;
    double posY;
    double depth;

    bool useStartAngle;
    double startAngle;

    bool useExitAngle;
    double exitAngle;
};
Q_DECLARE_METATYPE(Waypoint)

#endif // WAYPOINT_H
