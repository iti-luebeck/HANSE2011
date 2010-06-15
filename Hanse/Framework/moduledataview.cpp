#include "moduledataview.h"
#include "ui_moduledataview.h"

ModuleDataView::ModuleDataView(ModulesGraph *graph, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModuleDataView)
{
    ui->setupUi(this);

    this->graph = graph;

    dataModel = new DataModel(graph);

    ui->dataView->setModel(dataModel);

}

ModuleDataView::~ModuleDataView()
{
    delete ui;
    // TODO: delete model (or set this as parent)
}

void ModuleDataView::changeEvent(QEvent *e)
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

void ModuleDataView::on_filter_textChanged(QString filter)
{
    dataModel->setFilter(filter);
}
