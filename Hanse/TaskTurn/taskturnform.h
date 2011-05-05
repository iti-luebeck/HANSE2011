#ifndef TASKTTURNFORM_H
#define TASKTTURNFORM_H

#include <TaskTurn/taskturn.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskTurn;
class Module_Simulation;

namespace Ui {
    class TaskTurnForm;
}

class TaskTurnForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskTurnForm(TaskTurn *tt, QWidget *parent = 0);
    ~TaskTurnForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskTurnForm *ui;

    Log4Qt::Logger *logger;
    TaskTurn* taskturn;

signals:
    void newSchDesSignal(QString taskName, QString newD);

public slots:
    void on_applyButton_clicked();

};

#endif // TASKTTURNFORM_H
