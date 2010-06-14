#ifndef SCANNINGSONARFORM_H
#define SCANNINGSONARFORM_H

#include <QWidget>
#include "module_scanningsonar.h"
#include <QGraphicsScene>
#include <log4qt/logger.h>

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
    Log4Qt::Logger *logger;

    Ui::ScanningSonarForm *ui;
    Module_ScanningSonar* sonar;
    QGraphicsScene scene;
    QQueue<QGraphicsPolygonItem*> queue;
    QGraphicsItem* scanLine;
    float oldHeading;

private slots:
    void on_selFile_clicked();
    void on_fileReaderDelay_valueChanged(int );
    void on_fileCfgApply_clicked();
    void on_save_clicked();
    void updateSonarView(const SonarReturnData data);
};

#endif // SCANNINGSONARFORM_H
