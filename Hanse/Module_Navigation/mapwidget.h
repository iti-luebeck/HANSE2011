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

    QTimer t;

    QGraphicsItem *waypointsItem;
    QGraphicsItem *goalItem;

    QGraphicsItem* masterParticle;
    QGraphicsItem* masterMapPoint;
    QGraphicsItem* masterObsPoint;
    QGraphicsPixmapItem *satImage;
    QGraphicsEllipseItem *sonarPosition;
    QGraphicsLineItem *sonarPositionOrient;

    void stopSonarLocalization(QPointF point);
    void startSonarLocalization();
    bool isSonarLocalizationInProgress;

public slots:
    void graphicsMouseDoubleClicked( QPointF point );
    void graphicsMouseReleased( QPointF point );

//    void updateVisualSLAM();
    void updateWaypoints( QMap<QString, Waypoint> waypoints );
    void updateGoal( Waypoint goal );
    void clearGoal();

    void newSonarLocEstimate();
    void createMap();

private slots:
    void on_initialLocalizationBtn_clicked();
    void on_showVisSLAM_toggled(bool checked);
    void on_showParticles_toggled(bool checked);
    void on_showSatImg_toggled(bool checked);
    void on_showSonarObs_toggled(bool checked);
    void on_showSonarMap_toggled(bool checked);

    void timerElapsed();

signals:
    void newManualLocalization(QVector2D position);
};

#endif // MAPWIDGET_H
