#include "modulehealthview.h"
#include "ui_modulehealthview.h"

ModuleHealthView::ModuleHealthView(ModulesGraph *graph, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModuleHealthView)
{
    ui->setupUi(this);

    model = new HealthModel(graph);

    ui->healthView->setModel(model);
}

ModuleHealthView::~ModuleHealthView()
{
    delete ui;
    // TODO: delete model (or set this as parent)
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
