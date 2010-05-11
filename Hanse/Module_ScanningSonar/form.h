#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "module_scanningsonar.h"
#include <QGraphicsScene>

namespace Ui {
    class Form;
}

class Form : public QWidget {
    Q_OBJECT
public:
    Form(Module_ScanningSonar* sonar, QWidget *parent = 0);
    ~Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form *ui;
    Module_ScanningSonar* sonar;
    QGraphicsScene scene;
    QMap<double, QGraphicsItem*> map;
    QGraphicsItem* scanLine;


private slots:
    void on_fileCfgApply_clicked();
    void on_save_clicked();
    void updateSonarView(SonarReturnData data);
};

#endif // FORM_H
