#ifndef TESTTASKFORM_H
#define TESTTASKFORM_H

#include <TestTask/testtask.h>

#include <QWidget>
#include <log4qt/logger.h>

class TestTask;
class Module_Simulation;

namespace Ui {
    class TestTaskForm;
}

class TestTaskForm : public QWidget
{
    Q_OBJECT

public:
    explicit TestTaskForm(TestTask *tt, QWidget *parent = 0);
    ~TestTaskForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TestTaskForm *ui;

        Log4Qt::Logger *logger;
        TestTask* testtask;

signals:

public slots:

private slots:
        void on_startButton_clicked();
        void on_stopButton_clicked();
};

#endif // TESTTASKFORM_H
