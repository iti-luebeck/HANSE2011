#ifndef SCANNINGSONARFORM_H
#define SCANNINGSONARFORM_H

#include <QWidget>
#include "module_scanningsonar.h"
#include <QGraphicsScene>

namespace Ui {
    class ScanningSonarForm;
}

class ScanningSonarForm : public QWidget {
    Q_OBJECT
public:
    ScanningSonarForm(Module_ScanningSonar* sonar, QWidget *parent = 0);
    ~ScanningSonarForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ScanningSonarForm *ui;
    Module_ScanningSonar* sonar;
    QGraphicsScene scene;
    QMap<double, QGraphicsPolygonItem*> map;
    QGraphicsItem* scanLine;
    int oldHeading;


private slots:
    void on_fileCfgApply_clicked();
    void on_save_clicked();
    void updateSonarView(const SonarReturnData data);
};

#endif // SCANNINGSONARFORM_H
