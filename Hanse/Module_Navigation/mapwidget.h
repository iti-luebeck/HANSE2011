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

public slots:
//    void updateView( QGraphicsScene *scene );
    void graphicsMouseReleased( QPointF point );

    void newSonarLocEstimate();
};

#endif // MAPWIDGET_H
