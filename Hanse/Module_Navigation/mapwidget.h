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

    void createMap();

public slots:
    void graphicsMouseReleased( QPointF point );
    void updateVisualSLAM();
    void newSonarLocEstimate();
};

#endif // MAPWIDGET_H
