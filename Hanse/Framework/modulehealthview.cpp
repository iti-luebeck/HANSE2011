#include "modulehealthview.h"
#include "ui_modulehealthview.h"

ModuleHealthView::ModuleHealthView(ModulesGraph *graph, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModuleHealthView)
{
    ui->setupUi(this);

    model = new HealthModel(this, graph);

    ui->healthView->setModel(model);
}

ModuleHealthView::~ModuleHealthView()
{
    delete ui;
}

void ModuleHealthView::changeEvent(QEvent *e)
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
