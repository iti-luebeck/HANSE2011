#ifndef TESTTASK2FORM_H
#define TESTTASK2FORM_H

#include <TestTask2/testtask2.h>

#include <QWidget>
#include <log4qt/logger.h>

class TestTask2;
class Module_Simulation;

namespace Ui {
    class TestTask2Form;
}

class TestTask2Form : public QWidget
{
    Q_OBJECT

public:
    explicit TestTask2Form(TestTask2 *tt, QWidget *parent = 0);
    ~TestTask2Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TestTask2Form *ui;

        Log4Qt::Logger *logger;
        TestTask2* testtask2;

signals:

public slots:

private slots:
        void on_startButton_clicked();
        void on_stopButton_clicked();
};

#endif // TESTTASK2FORM_H
