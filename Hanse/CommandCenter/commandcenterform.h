#ifndef COMMANDCENTERFORM_H
#define COMMANDCENTERFORM_H

#include <CommandCenter/commandcenter.h>

#include <QWidget>
#include <log4qt/logger.h>

class CommandCenter;


namespace Ui {
    class CommandCenterForm;
}

class CommandCenterForm : public QWidget
{
    Q_OBJECT
public:
    CommandCenterForm(CommandCenter *command, QWidget *parent = 0);
    ~CommandCenterForm();

    //QList<QString> schedule;

protected:
    void changeEvent(QEvent *e);

private:
    Log4Qt::Logger *logger;
    Ui::CommandCenterForm *ui;
    CommandCenter* com;

signals:
    void startCommandCenter();
    void stopCommandCenter();

public slots:

private slots:
    void on_addButton_clicked();
    void on_revertButton_clicked();
    void on_clearButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();

    void updateTask(QString s);
    void updateError(QString s);
    void updateLists(QString s);
    void updateAborted(QString s);
};

#endif // COMMANDCENTERFORM_H
