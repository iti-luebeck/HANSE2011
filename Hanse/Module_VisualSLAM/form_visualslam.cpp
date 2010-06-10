#include "form_visualslam.h"
#include "ui_form_visualslam.h"

Form_VisualSLAM::Form_VisualSLAM( Module_VisualSLAM *visualSlam, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Form_VisualSLAM)
{
    ui->setupUi(this);
    this->visualSlam = visualSlam;
    QObject::connect( visualSlam, SIGNAL( updateFinished() ), SLOT( updateView() ) );
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

void Form_VisualSLAM::setScene( QGraphicsScene *scene )
{
    ui->graphicsView->setScene( scene );
}

void Form_VisualSLAM::updateView()
{
    QMutexLocker lockerScene( visualSlam->getSceneMutex() );
    ui->graphicsView->show();
}

void Form_VisualSLAM::on_startButton_clicked()
{
    visualSlam->start();
}

void Form_VisualSLAM::on_stopButton_clicked()
{
    visualSlam->stop();
}

void Form_VisualSLAM::on_zoomOutButton_clicked()
{
    ui->graphicsView->scale( 0.9, 0.9 );
    updateView();
}

void Form_VisualSLAM::on_zoomInButton_clicked()
{
    ui->graphicsView->scale( 1.1, 1.1 );
    updateView();
}

void Form_VisualSLAM::on_resetButton_clicked()
{
    visualSlam->reset();
}
