#include "mapwidget.h"
#include "ui_mapwidget.h"
#include <Module_Navigation/waypointdialog.h>

MapWidget::MapWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapWidget)
{
    ui->setupUi(this);

    QObject::connect( ui->graphicsView, SIGNAL( mouseEventAt(QPointF) ),
                      this, SLOT( graphicsMouseReleased(QPointF) ) );
    QGraphicsScene *scene = new QGraphicsScene( QRectF(), ui->graphicsView );
    ui->graphicsView->setScene( scene );
}

MapWidget::~MapWidget()
{
    delete ui;
}

void MapWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


QGraphicsScene *MapWidget::getGraphicsScene()
{
    return ui->graphicsView->scene();
}

void MapWidget::updateView( QGraphicsScene *scene )
{
    ui->graphicsView->scene()->clear();
    QList<QGraphicsItem *> items = scene->items();
    for ( int i = 0; i < items.size(); i++ )
    {
        ui->graphicsView->scene()->addItem( items[i] );
    }
    ui->graphicsView->show();
}


void MapWidget::graphicsMouseReleased( QPointF point )
{
    WaypointDialog wd( QString(), point.x(), -point.y(), 2.5, 0.0, 0.0, this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Position) ),
                      nav, SLOT( addWaypoint(QString,Position) ) );
    wd.exec();
}
