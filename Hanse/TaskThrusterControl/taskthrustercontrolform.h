#ifndef TASKTHRUSTERCONTROLFORM_H
#define TASKTHRUSTERCONTROLFORM_H

#include <TaskThrusterControl/taskthrustercontrol.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskThrusterControl;
class Module_Simulation;

namespace Ui {
    class TaskThrusterControlForm;
}

class TaskThrusterControlForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskThrusterControlForm(TaskThrusterControl *ttc, QWidget *parent = 0);
    ~TaskThrusterControlForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskThrusterControlForm *ui;

    Log4Qt::Logger *logger;
    TaskThrusterControl* taskthrustercontrol;

signals:
    void newSchDesSignal(QString taskName, QString newD);

public slots:
    void on_applyButton_clicked();

};

#endif // TASKTHRUSTERCONTROLFORM_H
