#include "mapwidget.h"
#include "ui_mapwidget.h"
#include <Module_Navigation/waypointdialog.h>
#include <Module_VisualSLAM/module_visualslam.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_SonarLocalization/sonarparticlefilter.h>

MapWidget::MapWidget( QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapWidget)
{
    ui->setupUi(this);

    QObject::connect( ui->graphicsView, SIGNAL( mouseEventAt(QPointF) ),
                      this, SLOT( graphicsMouseReleased(QPointF) ) );

    scene = new QGraphicsScene( QRectF(), ui->graphicsView );
    ui->graphicsView->setScene( scene );

    visualSLAMItem = NULL;
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
    connect( nav->visSLAM, SIGNAL(viewUpdated()), this, SLOT(updateVisualSLAM()) );

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

void MapWidget::graphicsMouseReleased( QPointF point )
{
    WaypointDialog wd( QString(), point.x(), -point.y(), 2.5, 0.0, 0.0, this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Position) ),
                      nav, SLOT( addWaypoint(QString,Position) ) );
    wd.exec();
}

void MapWidget::updateVisualSLAM()
{
    if ( visualSLAMItem != NULL )
    {
        delete( visualSLAMItem );
        visualSLAMItem = NULL;
    }

    if ( ui->showVisSLAM->isChecked() )
    {
        QBrush brush( Qt::blue );
        QPen pen( Qt::blue );

        QGraphicsScene *scene = ui->graphicsView->scene();
        visualSLAMItem = scene->addEllipse( 0, 0, 0, 0, pen, brush );

        QList<QPointF> landmarks;
        Position pos;
        nav->visSLAM->getPlotData( landmarks, pos );

        double width = 0.1;
        for ( int i = 0; i < landmarks.size(); i++ )
        {
            QGraphicsItem *item =
                    scene->addEllipse( landmarks[i].x() + width/4,
                                       -(landmarks[i].y() + width/4),
                                       width, width, pen, brush );
            item->setParentItem( visualSLAMItem );
            item->setZValue( 1000 );
        }

        brush = QBrush( Qt::red );
        pen = QPen( Qt::red );
        width = 0.3;
        QGraphicsItem *item =
                scene->addEllipse( pos.getX() + width/4,
                                   -(pos.getZ() + width/4),
                                   width, width, pen, brush );
        item->setParentItem( visualSLAMItem );
        item->setZValue( 1100 );

        ui->graphicsView->show();
    }
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
    result->setPos(0,0);
    result->setZValue(-1);
    result->setScale(0.2);// 5 px == 1m

    // draw map
    QVector<QVector2D> mapPoints = nav->sonarLoc->particleFilter().getMapPoints();
    foreach (QVector2D p, mapPoints) {
        scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("yellow")));
    }

    masterParticle = new QGraphicsLineItem();
    QVector<QVector4D> particles = nav->sonarLoc->particleFilter().getParticles();
    foreach (QVector4D p, particles) {
        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(p.x(), p.y(), 1,1, masterParticle);
        e->setPen(QPen(QColor("green")));
    }

}
