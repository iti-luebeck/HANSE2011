#ifndef MODULEHEALTHVIEW_H
#define MODULEHEALTHVIEW_H

#include <QWidget>
#include<Framework/modulesgraph.h>
#include <Framework/healthmodel.h>

namespace Ui {
    class ModuleHealthView;
}

class ModuleHealthView : public QWidget {
    Q_OBJECT
public:
    ModuleHealthView(ModulesGraph *graph, QWidget *parent);
    ~ModuleHealthView();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ModuleHealthView *ui;
    HealthModel *model;
};

#endif // MODULEHEALTHVIEW_H
