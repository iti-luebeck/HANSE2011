#include "form_visualslam.h"
#include "ui_form_visualslam.h"
#include <QMessageBox>

Form_VisualSLAM::Form_VisualSLAM( Module_VisualSLAM *visualSlam, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Form_VisualSLAM)
{
    ui->setupUi(this);
    this->visualSlam = visualSlam;
    QObject::connect( this, SIGNAL( changedSettings(double,double,double) ),
                      visualSlam, SLOT( changeSettings(double,double,double) ) );
}

Form_VisualSLAM::~Form_VisualSLAM()
{
    delete ui;
}

void Form_VisualSLAM::changeEvent(QEvent *e)
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

void Form_VisualSLAM::on_startButton_clicked()
{
    visualSlam->start();
}

void Form_VisualSLAM::on_stopButton_clicked()
{
    visualSlam->stop();
}

void Form_VisualSLAM::on_resetButton_clicked()
{
    visualSlam->reset();
}

void Form_VisualSLAM::on_applyButton_clicked()
{
    bool ok;
    double v_observation, v_translation, v_rotation;

    v_observation = ui->observationEdit->text().toDouble( ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect observation variance." );
        msgBox.exec();
        return;
    }

    v_translation = ui->translationEdit->text().toDouble( ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect observation variance." );
        msgBox.exec();
        return;
    }

    v_rotation = ui->rotationEdit->text().toDouble( ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect rotation variance." );
        msgBox.exec();
        return;
    }

    changedSettings( v_observation, v_translation, v_rotation );
}
