#ifndef TASKSURFACEFORM_H
#define TASKSURFACEFORM_H

#include <TaskSurface/tasksurface.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskSurface;
class Module_Simulation;

namespace Ui {
    class TaskSurfaceForm;
}

class TaskSurfaceForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskSurfaceForm(TaskSurface *tf, QWidget *parent = 0);
    ~TaskSurfaceForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskSurfaceForm *ui;

    Log4Qt::Logger *logger;
    TaskSurface* tasksurface;

signals:
    void updateTaskSettingsSignal();

public slots:
    void on_applyButton_clicked();

};

#endif // TASKSURFACEFORM_H
