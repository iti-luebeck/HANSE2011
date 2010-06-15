#ifndef MODULEDATAVIEW_H
#define MODULEDATAVIEW_H

#include <QWidget>
#include <QSettings>
#include <Framework/modulesgraph.h>
#include <Framework/datamodel.h>


namespace Ui {
    class ModuleDataView;
}

class ModuleDataView : public QWidget {
    Q_OBJECT
public:
    ModuleDataView(ModulesGraph *graph, QWidget *parent);
    ~ModuleDataView();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ModuleDataView *ui;
    ModulesGraph *graph;
    DataModel *dataModel;
    QSettings s;

private slots:
    void on_filter_textChanged(QString );
};

#endif // MODULEDATAVIEW_H
