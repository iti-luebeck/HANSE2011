#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QtGui>

#include <Module_Navigation/module_navigation.h>

namespace Ui {
    class MapWidget;
}

class MapWidget : public QWidget {
    Q_OBJECT
public:
    MapWidget(QWidget *parent = 0);
    ~MapWidget();

    QGraphicsScene* scene;

    void setNavigation(Module_Navigation* nav);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MapWidget *ui;
    Module_Navigation* nav;
    QGraphicsItem *visualSLAMItem;
    QGraphicsItem *waypointsItem;

    QGraphicsItem* masterParticle;
    QGraphicsItem* masterMapPoint;
    QGraphicsItem* masterObsPoint;
    QGraphicsPixmapItem *satImage;
    QGraphicsEllipseItem *sonarPosition;

    void createMap();

public slots:
    void graphicsMouseDoubleClicked( QPointF point );
    void graphicsMouseReleased( QPointF point );
    void updateVisualSLAM();
    void updateWaypoints( QMap<QString, Position> waypoints );
    void newSonarLocEstimate();

private slots:
    void on_showParticles_toggled(bool checked);
    void on_showSatImg_toggled(bool checked);
    void on_showSonarObs_toggled(bool checked);
    void on_showSonarMap_toggled(bool checked);
};

#endif // MAPWIDGET_H
