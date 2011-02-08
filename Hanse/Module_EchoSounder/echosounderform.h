#ifndef ECHOSOUNDERFORM_H
#define ECHOSOUNDERFORM_H

#include <QWidget>
#include "module_echosounder.h"
#include <QGraphicsScene>
#include <log4qt/logger.h>

namespace Ui {
    class EchoSounderForm;
}

class EchoSounderForm : public QWidget
{
    Q_OBJECT

public:
    EchoSounderForm(Module_EchoSounder* echo, QWidget *parent = 0);
    ~EchoSounderForm();

protected:
    void changeEvent(QEvent *e);


public slots:
    void updateSounderView(const EchoReturnData data);

private:
    Log4Qt::Logger *logger;

    Ui::EchoSounderForm *ui;
    Module_EchoSounder* echo;
    QGraphicsScene scene;
    QQueue<QLinearGradient> dataQueue;
    QQueue<QGraphicsRectItem*> ritQueue;

private slots:
    void on_save_clicked();
    void on_applyButton_clicked();
    void on_selFile_clicked();
};

#endif // ECHOSOUNDERFORM_H
