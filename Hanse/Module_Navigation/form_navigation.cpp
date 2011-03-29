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
    ui->mapWidget->setNavigation(nav);

//    QSettings& settings = nav->getSettings();
    ui->headingPEdit->setText( nav->getSettingsValue( QString( "p_heading" ),
                                               NAV_P_HEADING ).toString() );
    ui->headingHysteresisEdit->setText( nav->getSettingsValue( QString( "hysteresis_heading" ),
                                                        NAV_HYSTERESIS_HEADING ).toString() );
    ui->goalHysteresisEdit->setText( nav->getSettingsValue( QString( "hysteresis_goal" ),
                                                    NAV_HYSTERESIS_GOAL ).toString() );
    ui->depthHysteresisEdit->setText( nav->getSettingsValue( QString( "hysteresis_depth" ),
                                                      NAV_HYSTERESIS_DEPTH ).toString() );    
    ui->forwardSpeedEdit->setText( nav->getSettingsValue( "p_forward",
                                                   NAV_P_FORWARD).toString() );
    ui->forwardMaxDistEdit->setText( nav->getSettingsValue( "forward_max_dist",
                                                     NAV_FORWARD_MAX_DIST).toString() );
    ui->forwardMaxSpeedEdit->setText( nav->getSettingsValue( "forward_max_speed",
                                                      NAV_FORWARD_MAX_SPEED).toString() );
    ui->forwardTimeEdit->setText( nav->getSettingsValue( QString( "forward_time" ),
                                                  NAV_FORWARD_TIME).toString() );
    qRegisterMetaType<Position>("Position");
    connect(this,SIGNAL(goToPosition(QString,Position)),nav,SLOT(gotoWayPoint(QString,Position)));
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

void Form_Navigation::on_applyButton_clicked()
{
    bool ok;
    double p_heading = ui->headingPEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect heading P value." );
        msgBox.exec();
        return;
    }

    double hysteresis_heading = ui->headingHysteresisEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect heading hysteresis." );
        msgBox.exec();
        return;
    }

    double hysteresis_goal = ui->goalHysteresisEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect goal hysteresis." );
        msgBox.exec();
        return;
    }

    double hysteresis_depth = ui->depthHysteresisEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect depth hysteresis." );
        msgBox.exec();
        return;
    }

    double p_forward = ui->forwardSpeedEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect forward P value." );
        msgBox.exec();
        return;
    }

    double forward_time = ui->forwardTimeEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect forward time." );
        msgBox.exec();
        return;
    }

    double forward_max_speed = ui->forwardMaxSpeedEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect forward max speed." );
        msgBox.exec();
        return;
    }

    double forward_max_dist = ui->forwardMaxDistEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect forward max dist." );
        msgBox.exec();
        return;
    }

//    QSettings& settings = nav->getSettings();
    nav->setSettingsValue( "p_heading", p_heading );
    nav->setSettingsValue( "hysteresis_heading", hysteresis_heading );
    nav->setSettingsValue( "hysteresis_goal", hysteresis_goal );
    nav->setSettingsValue( "hysteresis_depth", hysteresis_depth );
    nav->setSettingsValue( "p_forward", p_forward );
    nav->setSettingsValue( "forward_time", forward_time );
    nav->setSettingsValue( "forward_max_speed", forward_max_speed );
    nav->setSettingsValue( "forward_max_dist", forward_max_dist );
    nav->setSettingsValue( "heading_sensor", ui->headingBox->currentIndex() );
}

void Form_Navigation::on_gotoButton_clicked()
{
    QList<QListWidgetItem *> selectedItems = ui->listWidget->selectedItems();
    qDebug() << "items " << selectedItems.size();
    if ( selectedItems.size() == 1 )
    {
        QString goal = selectedItems[0]->text();
        emit goToPosition(goal, Position());
//        nav->gotoWayPoint( goal, Position() );
    }
}

void Form_Navigation::on_clearGoalButton_clicked()
{
    nav->clearGoal();
}
