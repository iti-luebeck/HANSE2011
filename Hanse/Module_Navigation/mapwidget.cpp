#include "mapwidget.h"
#include "ui_mapwidget.h"
#include <Module_Navigation/waypointdialog.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>

MapWidget::MapWidget( QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapWidget)
{
    ui->setupUi(this);

    QObject::connect( ui->graphicsView, SIGNAL( mouseEventAt(QPointF) ),
                      this, SLOT( graphicsMouseReleased(QPointF) ) );

    scene = new QGraphicsScene( QRectF(), ui->graphicsView );
    ui->graphicsView->setScene( scene );

    nav = NULL;

}

MapWidget::~MapWidget()
{
    delete ui;
}

void MapWidget::setNavigation(Module_Navigation *nav)
{
    this->nav = nav;

    connect(nav->sonarLoc, SIGNAL(newLocalizationEstimate()), this, SLOT(newSonarLocEstimate()));

    createMap();
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

//void MapWidget::updateView( QGraphicsScene *scene )
//{
//    ui->graphicsView->scene()->clear();
//    QList<QGraphicsItem *> items = scene->items();
//    for ( int i = 0; i < items.size(); i++ )
//    {
//        ui->graphicsView->scene()->addItem( items[i] );
//    }
//    ui->graphicsView->show();
//}


void MapWidget::graphicsMouseReleased( QPointF point )
{
    WaypointDialog wd( QString(), point.x(), -point.y(), 2.5, 0.0, 0.0, this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Position) ),
                      nav, SLOT( addWaypoint(QString,Position) ) );
    wd.exec();
}

void MapWidget::newSonarLocEstimate()
{
    // TODO: fetch data from sonar loc module and put it in the scene
}

void MapWidget::createMap()
{
    ui->graphicsView->setScene(scene);

    QSettings s;
    s.beginGroup("sonarLocalize");
    QImage satImg(s.value("satImgFile").toString());
    QGraphicsPixmapItem *result = scene->addPixmap(QPixmap::fromImage(satImg));
    result->scale(5,5); // 1 unit == 5m
    result->setPos(0,0);
    result->setZValue(-1);
    result->setScale(0.2);

//    QVector<QVector4D> particles = m->pf->getParticles();
//    foreach (QVector4D p, particles) {
//        particleItems.append(scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("green"))));
//    }
//
//    // draw map
//    foreach (QVector2D p, m->pf->mapPoints) {
//        scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("yellow")));
//    }
}
