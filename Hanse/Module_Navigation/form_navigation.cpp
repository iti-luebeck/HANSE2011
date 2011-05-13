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

    ui->headingPEdit->setText(nav->getSettingsValue(QString("p_heading"), NAV_P_HEADING).toString());
    ui->headingHysteresisEdit->setText(nav->getSettingsValue(QString("hysteresis_heading"), NAV_HYSTERESIS_HEADING).toString());
    ui->goalHysteresisEdit->setText(nav->getSettingsValue(QString("hysteresis_goal"), NAV_HYSTERESIS_GOAL).toString());
    ui->depthHysteresisEdit->setText(nav->getSettingsValue(QString("hysteresis_depth"), NAV_HYSTERESIS_DEPTH).toString());
    ui->forwardSpeedEdit->setText(nav->getSettingsValue("p_forward", NAV_P_FORWARD).toString());
    ui->forwardMaxDistEdit->setText(nav->getSettingsValue("forward_max_dist", NAV_FORWARD_MAX_DIST).toString());
    ui->forwardMaxSpeedEdit->setText( nav->getSettingsValue("forward_max_speed", NAV_FORWARD_MAX_SPEED).toString());
    ui->forwardTimeEdit->setText(nav->getSettingsValue(QString("forward_time"), NAV_FORWARD_TIME).toString());
    ui->maxAngularSpeed->setText(nav->getSettingsValue("angular_max_speed").toString());
    ui->minAngularSpeed->setText(nav->getSettingsValue("angular_min_speed").toString());

    bool useCompass = nav->getSettingsValue("use compass", true).toBool();
    bool useXsens = nav->getSettingsValue("use xsens", false).toBool();

    if (useCompass || useXsens) {
        ui->headingBox->setChecked(true);
    } else {
        ui->headingBox->setChecked(false);
    }

    if (useXsens) {
        ui->xsensButton->setChecked(true);
    } else if (useCompass) {
        ui->compassButton->setChecked(true);
    }

    qRegisterMetaType<Position>("Position");

    QObject::connect(this,SIGNAL(goToPosition(QString)),nav,SLOT(gotoWayPoint(QString)));
    QObject::connect( nav, SIGNAL( updatedWaypoints(QMap<QString,Waypoint>) ),
                      this, SLOT( updateList(QMap<QString,Waypoint>) ) );
    QObject::connect( this, SIGNAL( removedWaypoint(QString) ),
                      nav, SLOT( removeWaypoint(QString) ) );
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
    if (waypoints.contains(name)) qDebug("contains name");
    Waypoint pos = waypoints[name];
    WaypointDialog wd( name, pos.posX, pos.posY, pos.depth, pos.useStartAngle,
                       pos.startAngle, pos.useExitAngle, pos.exitAngle, this );
    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Waypoint) ),
                      nav, SLOT( addWaypoint(QString,Waypoint) ) );
    wd.exec();
}

void Form_Navigation::on_addButton_clicked()
{
    WaypointDialog wd( QString(), 0.0, 0.0, 0.0, false, 0.0, false, 0.0, this );

    QObject::connect( &wd, SIGNAL( createdWaypoint(QString,Waypoint) ),
                      nav, SLOT( addWaypoint(QString,Waypoint) ) );
    wd.exec();
}


void Form_Navigation::updateList( QMap<QString, Waypoint> waypoints )
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
//    QString path = QFileDialog::getExistingDirectory( this, QString( "Choose map directory" ),
//                                                      QString( "maps" ) );
//    if ( !path.isEmpty() )
//    {
//        nav->save( path );
//    }
    nav->saveToSettings();
}

void Form_Navigation::on_pushButton_2_clicked()
{
//    QString path = QFileDialog::getExistingDirectory( this, QString( "Choose map directory" ),
//                                                      QString( "maps" ) );
//    if ( !path.isEmpty() )
//    {
//        nav->load( path );
//    }
    nav->loadFromSettings();
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

    float angular_max_speed = ui->maxAngularSpeed->text().toFloat( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect max. angular speed" );
        msgBox.exec();
        return;
    }

    float angular_min_speed = ui->minAngularSpeed->text().toFloat( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect min. angular speed" );
        msgBox.exec();
        return;
    }

    bool useCompass = false;
    bool useXsens = false;
    if (ui->headingBox->isChecked()) {
        if (ui->xsensButton->isChecked()) {
            useXsens = true;
        } else if (ui->compassButton->isChecked()) {
            useCompass = true;
        }
    }

    nav->setSettingsValue("p_heading", p_heading );
    nav->setSettingsValue("hysteresis_heading", hysteresis_heading );
    nav->setSettingsValue("hysteresis_goal", hysteresis_goal );
    nav->setSettingsValue("hysteresis_depth", hysteresis_depth );
    nav->setSettingsValue("p_forward", p_forward );
    nav->setSettingsValue("forward_time", forward_time );
    nav->setSettingsValue("forward_max_speed", forward_max_speed );
    nav->setSettingsValue("forward_max_dist", forward_max_dist );
    nav->setSettingsValue("angular_max_speed", angular_max_speed);
    nav->setSettingsValue("angular_min_speed", angular_min_speed);
    nav->setSettingsValue("use xsens", useXsens);
    nav->setSettingsValue("use compass", useCompass);
}

void Form_Navigation::on_gotoButton_clicked()
{
    QList<QListWidgetItem *> selectedItems = ui->listWidget->selectedItems();
    qDebug() << "items " << selectedItems.size();
    if ( selectedItems.size() == 1 )
    {
        QString goal = selectedItems[0]->text();
        emit goToPosition(goal);
    }
}

void Form_Navigation::on_clearGoalButton_clicked()
{
    QTimer::singleShot(0, nav, SLOT(clearGoal()));
}

void Form_Navigation::on_pauseButton_clicked()
{
    QTimer::singleShot(0, nav, SLOT(pause()));
}

void Form_Navigation::on_resumeButton_clicked()
{
    QTimer::singleShot(0, nav, SLOT(resume()));
}
