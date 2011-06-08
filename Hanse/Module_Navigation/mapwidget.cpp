#include "mapwidget.h"
#include "ui_mapwidget.h"
#include <Module_Navigation/waypointdialog.h>
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


    scene = new QGraphicsScene( QRectF(-1000,-1000,2000,2000), ui->graphicsView );
    ui->graphicsView->setScene( scene );

    isSonarLocalizationInProgress = false;

    waypointsItem = NULL;
    goalItem = NULL;
    nav = NULL;

    masterMapPoint = NULL;
    masterObsPoint = NULL;
    satImage = NULL;
    masterParticle = NULL;
    sonarPosition = NULL;
    sonarPositionOrient = NULL;
}

MapWidget::~MapWidget()
{
    delete ui;
}

void MapWidget::setNavigation(Module_Navigation *nav)
{
    this->nav = nav;

    QObject::connect(this,SIGNAL(newManualLocalization(QVector2D)),nav->sonarLoc,SLOT(setLocalization(QVector2D)));

    createMap();

    // TODO: somehow doesn't work
    ui->graphicsView->setTransform(nav->getSettingsValue("mapPosition").value<QTransform>());

    ui->showParticles->setChecked(nav->getSettingsValue("showParticles").toBool());
    ui->showSatImg->setChecked(nav->getSettingsValue("showSatImg").toBool());
    ui->showSonarMap->setChecked(nav->getSettingsValue("showSonarMap").toBool());
    ui->showSonarObs->setChecked(nav->getSettingsValue("showSonarObs").toBool());
    ui->showVisSLAM->setChecked(nav->getSettingsValue("showVisSLAM").toBool());

    connect( nav->sonarLoc, SIGNAL(newLocalizationEstimate()), this, SLOT(newSonarLocEstimate()));
    connect( nav, SIGNAL(updatedWaypoints(QMap<QString,Waypoint>)), this, SLOT(updateWaypoints(QMap<QString,Waypoint>)) );
    connect( nav, SIGNAL(setNewGoal(Waypoint)), this, SLOT(updateGoal(Waypoint)) );
    connect( nav, SIGNAL(clearedGoal()), this, SLOT(clearGoal()) );

    // save current pos twice a second
    connect(&t, SIGNAL(timeout()), this, SLOT(timerElapsed()));
    t.start(500);

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
    WaypointDialog wd( QString(), point.x(), point.y(), 0.0, false, 0.0, false, 0.0, this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Waypoint) ),
                      nav, SLOT( addWaypoint(QString,Waypoint) ) );
    wd.exec();
}

void MapWidget::graphicsMouseReleased( QPointF point )
{
    if (isSonarLocalizationInProgress)
        stopSonarLocalization(point);
}

void MapWidget::updateWaypoints( QMap<QString, Waypoint> waypoints )
{
    if ( waypointsItem != NULL )
    {
        delete( waypointsItem );
        waypointsItem = NULL;
    }

    QBrush brush( Qt::green );
    QPen pen(QBrush(Qt::white), 0.5);
    QGraphicsScene *scene = ui->graphicsView->scene();
    waypointsItem = scene->addEllipse( 0, 0, 0, 0, pen, brush );
    waypointsItem->setZValue( 1200 );

    QList<QString> waypointNames = waypoints.keys();
    double width = 3;
    for ( int i = 0; i < waypointNames.size(); i++ )
    {
        Waypoint pos = waypoints[waypointNames[i]];
        QGraphicsItem *item =
                scene->addRect( pos.posX - width/2,
                                pos.posY - width/2,
                                width, width, pen, brush );
        item->setParentItem( waypointsItem );
        item->setZValue( 1220 );

        if (pos.useExitAngle) {
            QGraphicsItem *line =
                    scene->addLine(pos.posX, pos.posY, pos.posX - 5 * sin(pos.exitAngle * M_PI / 180),
                                   pos.posY + 5 * cos(pos.exitAngle * M_PI / 180), pen);
            line->setParentItem( waypointsItem );
            line->setZValue( 1221 );
        }

        QFont f( "Arial", 15 * width );
        QGraphicsTextItem *textItem = scene->addText( waypointNames[i], f );
        textItem->setParentItem( waypointsItem );
        textItem->setZValue( 1210 );
        textItem->setPos( pos.posX + width / 1.5, pos.posY - width / 2 );
        textItem->scale( 0.1, 0.1 );
        textItem->setDefaultTextColor( Qt::white );
    }
}

void MapWidget::updateGoal( Waypoint goal )
{
    if ( goalItem != NULL )
    {
        delete( goalItem );
        goalItem = NULL;
    }

    QPen pen( Qt::yellow );
    QBrush brush;
    pen.setWidthF( 0.2 );

    QGraphicsScene *scene = ui->graphicsView->scene();
    goalItem = scene->addEllipse( 0, 0, 0, 0, pen, brush );
    goalItem->setZValue( 1150 );


    Position pos = nav->getCurrentPosition();

    QGraphicsLineItem *path = new QGraphicsLineItem(goal.posX, goal.posY, pos.getX(),
            pos.getY(), masterParticle);
    path->setPen(QPen(QBrush(QColor(150,150,0)), 0.5));
    path->setZValue( 1000 );

    ui->graphicsView->show();
}

void MapWidget::clearGoal()
{
    if ( goalItem != NULL )
    {
        delete( goalItem );
        goalItem = NULL;
    }
}

void MapWidget::newSonarLocEstimate()
{
    createMap();

    delete masterParticle;
    masterParticle = scene->addLine(0,0,0,0);
    masterParticle->setVisible(ui->showParticles->isChecked());
    QVector<QVector4D> particles = nav->sonarLoc->particleFilter().getParticles();
    qreal maxWeight = 0;
    foreach (QVector4D p, particles) {
        if (p.w() > maxWeight) maxWeight = p.w();
    }

    foreach (QVector4D p, particles) {
        double width = 1.5;
        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(p.x(), p.y(), width, width, masterParticle);
        qreal pint = 255 * (p.w() / maxWeight);
        int weight = (int) pint;

        if (weight < 0) {
            weight = 0;
            qDebug() << "neg weight";
        }
        if (weight > 255) {
            weight = 255;
        }
        e->setPen(Qt::NoPen);
        e->setBrush(QBrush(QColor(weight, 255 - weight/2, 0)));
        e->setZValue(weight + 5);
    }

    QVector4D particle = particles[0];
    sonarPosition->setRect(particle.x() - 1, particle.y() - 1, 2, 2);
    sonarPositionOrient->setLine(particle.x(), particle.y(),
                                 particle.x() - 8 * sin( particle.z() ),
                                 particle.y() + 8 * cos( particle.z() )
                                 );

    delete masterObsPoint;
    masterObsPoint = scene->addLine(0,0,0,0);
    masterObsPoint->setVisible(ui->showSonarObs->isChecked());
    QList<QVector2D> zList = nav->sonarLoc->particleFilter().getLatestObservation();
//    zList.append(QVector2D(1,1));
    foreach (QVector2D o, zList) {
        double width = 1.5;
        QTransform rotM = QTransform().rotate(particle.z()/M_PI*180) * QTransform().translate(particle.x(), particle.y());
        QPointF q = rotM.map(o.toPointF());

        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(q.x(), q.y(), width, width, masterObsPoint);
        e->setPen(Qt::NoPen);
        e->setBrush(QBrush(QColor(0,210,255)));
    }

    if (nav->hasGoal()) {
        Waypoint goal = nav->getCurrentGoal();
        Position pos = nav->getCurrentPosition();

        QGraphicsLineItem *path = new QGraphicsLineItem(goal.posX, goal.posY, pos.getX(),
                pos.getY(), masterParticle);
        path->setPen(QPen(QBrush(QColor(200,200,0)), 0.5));
    }
}

void MapWidget::createMap()
{
    ui->graphicsView->setScene(scene);

    QSettings s(QSettings::IniFormat, QSettings::UserScope,"HanseCfg", "sonarLocalize");

    if (satImage) delete(satImage);
    if (masterObsPoint) delete(masterObsPoint);
    if (masterMapPoint) delete(masterMapPoint);
    if (masterParticle) delete(masterParticle);
    if (sonarPosition) delete(sonarPosition);
    if (sonarPositionOrient) delete(sonarPositionOrient);

    QImage satImg(s.value("satImgFile").toString());
    satImage = scene->addPixmap(QPixmap::fromImage(satImg));
    satImage->setPos(0,0);
    satImage->setZValue(-1);
    satImage->setScale(0.2);// 5 px == 1m

    masterObsPoint = scene->addLine(0,0,0,0);

    // draw map
    masterMapPoint = scene->addLine(0,0,0,0);
    QList<QVector2D> mapPoints = nav->sonarLoc->particleFilter().getMapPoints();
    foreach (QVector2D p, mapPoints) {
        double width = 1.5;
        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(p.x(), p.y(), width, width, masterMapPoint);
        e->setPen(Qt::NoPen);
        e->setBrush(QBrush(Qt::yellow));
        e->setZValue(0);
    }

    masterParticle = scene->addLine(0,0,0,0);
    QVector<QVector4D> particles = nav->sonarLoc->particleFilter().getParticles();
    foreach (QVector4D p, particles) {
        double width = 1;
        QGraphicsEllipseItem *e = new QGraphicsEllipseItem(p.x(), p.y(), width, width, masterParticle);
        e->setPen(Qt::NoPen);
        e->setBrush(QBrush(Qt::green));
        e->setZValue(2);
    }

    sonarPosition = scene->addEllipse(particles[0].x() - 1, particles[0].y() - 1, 2, 2, Qt::NoPen);
    sonarPositionOrient = scene->addLine( particles[0].x(), particles[0].y(),
                           particles[0].x() - 8 * sin( particles[0].z() ),
                           particles[0].y() + 8 * cos( particles[0].z() ),
                           QPen(QBrush(Qt::red), 1) );
    sonarPosition->setZValue(300);
    sonarPosition->setBrush(QBrush(Qt::red));
    sonarPositionOrient->setZValue(300);

}

void MapWidget::on_showSonarMap_toggled(bool checked)
{
    masterMapPoint->setVisible(checked);
    if (nav)
        nav->setSettingsValue("showSonarMap", checked);
}

void MapWidget::on_showSonarObs_toggled(bool checked)
{
    masterObsPoint->setVisible(checked);
    if (nav)
        nav->setSettingsValue("showSonarObs", checked);
}

void MapWidget::on_showSatImg_toggled(bool checked)
{
    satImage->setVisible(checked);
    if (nav)
        nav->setSettingsValue("showSatImg", checked);
}

void MapWidget::on_showParticles_toggled(bool checked)
{
    masterParticle->setVisible(checked);
    if (nav)
        nav->setSettingsValue("showParticles", checked);
}

void MapWidget::on_showVisSLAM_toggled(bool checked)
{
    if (nav)
        nav->setSettingsValue("showVisSLAM", checked);
}

void MapWidget::on_initialLocalizationBtn_clicked()
{
    startSonarLocalization();
}

void MapWidget::startSonarLocalization()
{
    if (!isSonarLocalizationInProgress)
        QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));

    isSonarLocalizationInProgress = true;
}

void MapWidget::stopSonarLocalization(QPointF point)
{
//    nav->sonarLoc->setLocalization(QVector2D(point));
    emit newManualLocalization(QVector2D(point));
    QApplication::restoreOverrideCursor();
    isSonarLocalizationInProgress = false;
}

void MapWidget::timerElapsed()
{
    nav->setSettingsValue("mapPosition", ui->graphicsView->transform());
}
