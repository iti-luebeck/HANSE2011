#include "metabehaviourform.h"
#include "ui_metabehaviourform.h"
#include <MetaBehaviour/metabehaviour.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>

MetaBehaviourForm::MetaBehaviourForm(MetaBehaviour* meta, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MetaBehaviourForm)
{
    ui->setupUi(this);

    this->meta = meta;

    foreach (RobotBehaviour* b, meta->behaviours) {
        QPushButton* p = new QPushButton(b->getId(), ui->behavioursWidget);
        connect(p, SIGNAL(clicked()), &signalMapperClicked, SLOT(map()));
        signalMapperClicked.setMapping(p, b);
        ui->formLayout->addWidget(p);

        QLabel* label = new QLabel("stopped", ui->behavioursWidget);
        ui->formLayout->addWidget(label);
        mapLabels[b]=label;

        connect(b, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(moduleFinished(RobotBehaviour*,bool)));
        connect(b, SIGNAL(started(RobotBehaviour*)), this, SLOT(moduleStarted(RobotBehaviour*)));
        connect(b, SIGNAL(healthStatusChanged(RobotModule*)), this, SLOT(moduleHealthFail(RobotModule*)));
    }

    connect(&signalMapperClicked, SIGNAL(mapped(QObject*)),
            this, SLOT(activateModule(QObject*)));


}

MetaBehaviourForm::~MetaBehaviourForm()
{
    delete ui;
}

void MetaBehaviourForm::changeEvent(QEvent *e)
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

// TODO: put some shortcut on this?
void MetaBehaviourForm::on_stopBehaviours_clicked()
{
    foreach (RobotBehaviour* b, meta->behaviours) {
        b->stop();
    }
    meta->tcl->reset();
}

void MetaBehaviourForm::activateModule(QObject *o) {
    RobotBehaviour* thisB = dynamic_cast<RobotBehaviour*>(o);
    if (thisB) {
        meta->logger->info("Starting module "+thisB->getId());
        foreach (RobotBehaviour* b, meta->behaviours) {
            if (b != thisB)
                b->stop();
        }
        thisB->start();
    }
}

void MetaBehaviourForm::moduleFinished(RobotBehaviour *module, bool success)
{
    mapLabels[module]->setText(success?"stopped: success":"stopped: failed");
}

void MetaBehaviourForm::moduleStarted(RobotBehaviour *module)
{
    mapLabels[module]->setText("active");
}

void MetaBehaviourForm::moduleHealthFail(RobotModule *module)
{
    meta->logger->info("Stopping module "+module->getId()+" because of health fail.");
    if (!module->getHealthStatus().isHealthOk()) {
        ((RobotBehaviour*)module)->stop();
        moduleFinished((RobotBehaviour*)module, false);
    }
}
