#include "moduledataview.h"
#include "ui_moduledataview.h"

ModuleDataView::ModuleDataView(ModulesGraph *graph, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModuleDataView),
    s(QSettings::IniFormat, QSettings::UserScope,"HanseCfg", "gui_docks")
{
    ui->setupUi(this);

    this->graph = graph;

    dataModel = new DataModel(this, graph);

    ui->dataView->setModel(dataModel);

    QStringList ds = s.value("openDataWidgets").toStringList();
    ds.append(parent->objectName());
    ds.removeDuplicates();
    s.setValue("openDataWidgets",ds);

    QString filter = s.value(parent->objectName()).toString();
    dataModel->setFilter(filter);
    ui->filter->setText(filter);

}

ModuleDataView::~ModuleDataView()
{
    delete ui;
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
    s.setValue(parent()->objectName(),filter);
}
