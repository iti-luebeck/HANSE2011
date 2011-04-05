#ifndef COMMANDCENTERFORM_H
#define COMMANDCENTERFORM_H

#include <CommandCenter/commandcenter.h>

#include <QWidget>
#include <log4qt/logger.h>

class CommandCenter;
class Module_Simulation;

namespace Ui {
    class CommandCenterForm;
}

class CommandCenterForm : public QWidget
{
    Q_OBJECT

public:

    CommandCenterForm(CommandCenter *command, QWidget *parent = 0);
    ~CommandCenterForm();

protected:
    void changeEvent(QEvent *e);

private:
    Log4Qt::Logger *logger;

    Ui::CommandCenterForm *ui;
    CommandCenter* command;

signals:

public slots:

private slots:

};

#endif // COMMANDCENTERFORM_H
