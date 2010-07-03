#ifndef METABEHAVIOURFORM_H
#define METABEHAVIOURFORM_H

#include <QWidget>
#include <QtGui>

#include "metabehaviour.h"

namespace Ui {
    class MetaBehaviourForm;
}

class MetaBehaviourForm : public QWidget {
    Q_OBJECT
public:
    MetaBehaviourForm(MetaBehaviour* meta, QWidget *parent = 0);
    ~MetaBehaviourForm();

protected:
    void changeEvent(QEvent *e);

private:
    MetaBehaviour* meta;
    Ui::MetaBehaviourForm *ui;
    QSignalMapper signalMapperClicked;
    QMap<RobotBehaviour*, QLabel*> mapLabels;

private slots:
    void on_pipeFollowNoDepthButton_clicked();
    void on_pipeFollowMeta_clicked();
    void on_stopBehaviours_clicked();
    void activateModule(QObject *o);
    void moduleStarted(RobotBehaviour* module);
    void moduleFinished(RobotBehaviour* module, bool success);

    void moduleHealthFail(RobotModule* module);

signals:
    void testPipe();

};

#endif // METABEHAVIOURFORM_H
