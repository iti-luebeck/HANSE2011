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

    QObject::connect( ui->graphicsView, SIGNAL( mouseDoubleClickEventAt(QPointF) ),
                      this, SLOT( graphicsMouseDoubleClicked(QPointF) ) );
    QObject::connect( ui->graphicsView, SIGNAL( mouseReleaseEventAt(QPointF) ),
                      this, SLOT( graphicsMouseReleased(QPointF)) );

    scene = new QGraphicsScene( QRectF(), ui->graphicsView );
    ui->graphicsView->setScene( scene );

    visualSLAMItem = NULL;
    waypointsItem = NULL;
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
    connect( nav, SIGNAL(updatedWaypoints(QMap<QString,Position>)), this, SLOT(updateWaypoints(QMap<QString,Position>)) );

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

void MapWidget::graphicsMouseDoubleClicked( QPointF point )
{
    WaypointDialog wd( QString(), point.x(), -point.y(), 2.5, 0.0, 0.0, this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Position) ),
                      nav, SLOT( addWaypoint(QString,Position) ) );
    wd.exec();
}

void MapWidget::graphicsMouseReleased( QPointF point )
{
    //TODO Initiale Lokalisation.
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
        visualSLAMItem->setZValue( 1000 );

        QList<QPointF> landmarks;
        Position pos;
        nav->visSLAM->getPlotData( landmarks, pos );

        double width = 0.1;
        for ( int i = 0; i < landmarks.size(); i++ )
        {
            QGraphicsItem *item =
                    scene->addEllipse( landmarks[i].x() - width/2,
                                       -landmarks[i].y() - width/2,
                                       width, width, pen, brush );
            item->setParentItem( visualSLAMItem );
            item->setZValue( 1000 );
        }

        brush = QBrush( Qt::red );
        pen = QPen( Qt::white );
        width = 0.3;
        QGraphicsItem *item =
                scene->addEllipse( pos.getX() - width/2,
                                   -pos.getZ() - width/2,
                                   width, width, pen, brush );
        item->setParentItem( visualSLAMItem );
        item->setZValue( 1100 );
        pen = QPen( Qt::red );
        item = scene->addLine( pos.getX(), -pos.getZ(),
                               pos.getX() + sin( pos.getYaw() * CV_PI / 180 ),
                               -(pos.getZ() + cos( pos.getYaw() * CV_PI / 180 )),
                               pen );
        item->setParentItem( visualSLAMItem );
        item->setZValue( 1100 );

        ui->graphicsView->show();
    }
}

void MapWidget::updateWaypoints( QMap<QString, Position> waypoints )
{
    if ( waypointsItem != NULL )
    {
        delete( waypointsItem );
        waypointsItem = NULL;
    }

    QBrush brush( Qt::green );
    QPen pen( Qt::white );
    QGraphicsScene *scene = ui->graphicsView->scene();
    waypointsItem = scene->addEllipse( 0, 0, 0, 0, pen, brush );
    waypointsItem->setZValue( 1200 );

    QList<QString> waypointNames = waypoints.keys();
    double width = 0.5;
    for ( int i = 0; i < waypointNames.size(); i++ )
    {
        Position pos = waypoints[waypointNames[i]];
        QGraphicsItem *item =
                scene->addRect( pos.getX() - width/2,
                                -pos.getY() - width/2,
                                width, width, pen, brush );
        item->setParentItem( waypointsItem );
        item->setZValue( 1200 );

        QFont f( "Arial", 12 );
        QGraphicsTextItem *textItem = scene->addText( waypointNames[i], f );
        textItem->setParentItem( waypointsItem );
        textItem->setZValue( 1210 );
        textItem->setPos( pos.getX() + width / 2, -pos.getY() );
        textItem->scale( 0.02, 0.02 );
    }
}

void MapWidget::newSonarLocEstimate()
{
    delete masterParticle;
    masterParticle = scene->addLine(0,0,0,0);
    masterParticle->setVisible(ui->showParticles->isChecked());
    QVector<QVector4D> particles = nav->sonarLoc->particleFilter().getParticles();
    foreach (QVector4D p, particles) {
        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(p.x(), p.y(), 1,1, masterParticle);
        e->setPen(QPen(QColor("green")));
    }

    QVector4D particle = particles[0];
    //sonarPosition->setPos(particle.toPointF());
    sonarPosition->setRect(particle.x(), particle.y(), 1,1);

    delete masterObsPoint;
    masterObsPoint = scene->addLine(0,0,0,0);
    masterObsPoint->setVisible(ui->showSonarObs->isChecked());
    QList<QVector2D> zList = nav->sonarLoc->particleFilter().getLatestObservation();
    zList.append(QVector2D(1,1));
    foreach (QVector2D o, zList) {

        QTransform rotM = QTransform().rotate(particle.z()/M_PI*180) * QTransform().translate(particle.x(), particle.y());
        QPointF q = rotM.map(o.toPointF());

        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(q.x(), q.y(), 1,1, masterObsPoint);
        e->setPen(QPen(QColor("blue")));
    }

}

void MapWidget::createMap()
{
    ui->graphicsView->setScene(scene);

    QSettings s;
    s.beginGroup("sonarLocalize");
    QImage satImg(s.value("satImgFile").toString());
    satImage = scene->addPixmap(QPixmap::fromImage(satImg));
    satImage->setPos(0,0);
    satImage->setZValue(-1);
    satImage->setScale(0.2);// 5 px == 1m

    masterObsPoint = scene->addLine(0,0,0,0);

    // draw map
    masterMapPoint = scene->addLine(0,0,0,0);
    QVector<QVector2D> mapPoints = nav->sonarLoc->particleFilter().getMapPoints();
    foreach (QVector2D p, mapPoints) {
        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(p.x(), p.y(), 1,1, masterMapPoint);
        e->setPen(QPen(QColor("yellow")));
        e->setZValue(0);
    }

    masterParticle = scene->addLine(0,0,0,0);
    QVector<QVector4D> particles = nav->sonarLoc->particleFilter().getParticles();
    foreach (QVector4D p, particles) {
        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(p.x(), p.y(), 1,1, masterParticle);
        e->setPen(QPen(QColor("green")));
        e->setZValue(2);
    }

    sonarPosition = scene->addEllipse(particles[0].x(), particles[0].y(), 1,1, QPen(QColor("red")));
    sonarPosition->setZValue(3);

}

void MapWidget::on_showSonarMap_toggled(bool checked)
{
    masterMapPoint->setVisible(checked);
}

void MapWidget::on_showSonarObs_toggled(bool checked)
{
    masterObsPoint->setVisible(checked);
}

void MapWidget::on_showSatImg_toggled(bool checked)
{
    satImage->setVisible(checked);
}

void MapWidget::on_showParticles_toggled(bool checked)
{
    masterParticle->setVisible(checked);
}
