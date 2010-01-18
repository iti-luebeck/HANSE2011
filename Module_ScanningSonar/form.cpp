#include "form.h"
#include "ui_form.h"
#include <sonarreturndata.h>

Form::Form(Module_ScanningSonar* sonar, QWidget *parent) :
        QWidget(parent), scene(-500,-500,1000,1000),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    this->sonar = sonar;

    this->ui->graphicsView->setScene(&scene);

    connect(sonar, SIGNAL(newSonarData()), this, SLOT(updateSonarView()));

}

Form::~Form()
{
    delete ui;
}

void Form::changeEvent(QEvent *e)
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

void Form::updateSonarView()
{
    SonarReturnData* data = sonar->data[sonar->data.length()-1];
    QGraphicsItem* it = scene.addText("");

    int th = 50;
    int r = data->getRange();
    for (int i = 0; i < data->getEchoData().length(); ++i) {
        char b = data->getEchoData()[i];
        if (b>th)
            QGraphicsEllipseItem *point = new QGraphicsEllipseItem(0,i, 1,1,it);
    }
    it->setRotation(data->getHeadPosition());
}
