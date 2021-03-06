#include "waypointdialog.h"
#include "ui_waypointdialog.h"
#include <QMessageBox>

WaypointDialog::WaypointDialog(QString name, double x, double y, double depth, bool useStartAngle,
                               double startAngle, bool useExitAngle, double exitAngle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WaypointDialog)
{
    ui->setupUi(this);
    ui->nameEdit->setText( name );
    ui->xEdit->setText( QString( "%1" ).arg( x, 0, 'f', -1 ) );
    ui->yEdit->setText( QString( "%1" ).arg( y, 0, 'f', -1 ) );
    ui->depthEdit->setText( QString( "%1" ).arg( depth, 0, 'f', -1 ) );
    ui->arrivalAngleEdit->setText( QString( "%1" ).arg( startAngle, 0, 'f', -1 ) );
    ui->exitAngleEdit->setText( QString( "%1" ).arg( exitAngle, 0, 'f', -1 ) );
    ui->useStartBox->setChecked(useStartAngle);
    ui->useExitBox->setChecked(useExitAngle);
}

WaypointDialog::~WaypointDialog()
{
    delete ui;
}

void WaypointDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void WaypointDialog::on_buttonBox_accepted()
{
    bool ok;

    QString name = ui->nameEdit->text();

    double x = ui->xEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect X position." );
        msgBox.exec();
        return;
    }

    double y = ui->yEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect Y position." );
        msgBox.exec();
        return;
    }

    double depth = ui->depthEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect depth." );
        msgBox.exec();
        return;
    }

    double arrivalAngle = ui->arrivalAngleEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect arrival angle." );
        msgBox.exec();
        return;
    }

    double exitAngle = ui->exitAngleEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect exit angle." );
        msgBox.exec();
        return;
    }

    bool useStartAngle = ui->useStartBox->isChecked();
    bool useExitAngle = ui->useExitBox->isChecked();

    Waypoint waypoint(name, x, y, depth, useStartAngle, arrivalAngle, useExitAngle, exitAngle);

    emit createdWaypoint( name, waypoint );
}
