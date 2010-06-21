#include "form_navigation.h"
#include "ui_form_navigation.h"
#include <Module_Navigation/waypointdialog.h>
#include <QFileDialog>

Form_Navigation::Form_Navigation( Module_Navigation *nav, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Form_Navigation)
{
    ui->setupUi(this);
    this->nav = nav;
    QObject::connect( ui->graphicsView, SIGNAL( mouseEventAt(QPointF) ),
                      this, SLOT( graphicsMouseReleased(QPointF) ) );
    QGraphicsScene *scene = new QGraphicsScene( QRectF(), ui->graphicsView );
    ui->graphicsView->setScene( scene );
}

Form_Navigation::~Form_Navigation()
{
    delete ui;
}

void Form_Navigation::changeEvent(QEvent *e)
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

void Form_Navigation::on_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
    QString name = item->text();
    Position pos = waypoints[ name ];
    WaypointDialog wd( name, pos.getX(), pos.getY(), pos.getDepth(), pos.getArrivalAngle(),
                       pos.getExitAngle(), this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Position) ),
                      nav, SLOT( addWaypoint(QString,Position) ) );
    wd.exec();
}

void Form_Navigation::on_addButton_clicked()
{
    WaypointDialog wd( QString(), 0.0, 0.0, 2.5, 0.0, 0.0, this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Position) ),
                      nav, SLOT( addWaypoint(QString,Position) ) );
    wd.exec();
}

void Form_Navigation::graphicsMouseReleased( QPointF point )
{
    WaypointDialog wd( QString(), point.x(), -point.y(), 2.5, 0.0, 0.0, this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Position) ),
                      nav, SLOT( addWaypoint(QString,Position) ) );
    wd.exec();
}

void Form_Navigation::updateList( QMap<QString, Position> waypoints )
{
    this->waypoints = waypoints;
    ui->listWidget->clear();
    QList<QString> waypointsList = waypoints.keys();
    for ( int i = 0; i < waypointsList.size(); i++ )
    {
        ui->listWidget->addItem( waypointsList[i] );
    }
}

void Form_Navigation::on_removeButton_clicked()
{
    QList<QListWidgetItem *> selectedItems = ui->listWidget->selectedItems();
    for ( int i = 0; i < selectedItems.size(); i++ )
    {
        removedWaypoint( selectedItems[i]->text() );
    }
}

QGraphicsScene *Form_Navigation::getGraphicsScene()
{
    return ui->graphicsView->scene();
}

void Form_Navigation::updateView( QGraphicsScene *scene )
{
    ui->graphicsView->scene()->clear();
    QList<QGraphicsItem *> items = scene->items();
    for ( int i = 0; i < items.size(); i++ )
    {
        ui->graphicsView->scene()->addItem( items[i] );
    }
    ui->graphicsView->show();
}

void Form_Navigation::on_pushButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory( this, QString( "Choose map directory" ),
                                                      QString( "maps" ) );
    if ( !path.isEmpty() )
    {
        nav->save( path );
    }
}

void Form_Navigation::on_pushButton_2_clicked()
{
    QString path = QFileDialog::getExistingDirectory( this, QString( "Choose map directory" ),
                                                      QString( "maps" ) );
    if ( !path.isEmpty() )
    {
        nav->load( path );
    }
}
