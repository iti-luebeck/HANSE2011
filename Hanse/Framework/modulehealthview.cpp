#include "modulehealthview.h"
#include "ui_modulehealthview.h"

ModuleHealthView::ModuleHealthView(ModulesGraph *graph, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModuleHealthView),
    s(QSettings::IniFormat, QSettings::UserScope,"HanseCfg", "gui_docks")
{
    ui->setupUi(this);

    model = new HealthModel(this, graph);

    QStringList ds = s.value("openHealthWidgets").toStringList();
    ds.append(parent->objectName());
    ds.removeDuplicates();
    s.setValue("openHealthWidgets",ds);

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
