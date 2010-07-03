#ifndef FORM_TURNONEEIGHTY_H
#define FORM_TURNONEEIGHTY_H

#include <QWidget>

class Behaviour_TurnOneEighty;

namespace Ui {
    class Form_TurnOneEighty;
}

class Form_TurnOneEighty : public QWidget {
    Q_OBJECT
public:
    Form_TurnOneEighty( Behaviour_TurnOneEighty *behaviour, QWidget *parent = 0 );
    ~Form_TurnOneEighty();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_TurnOneEighty *ui;
    Behaviour_TurnOneEighty *behaviour;

private slots:
    void on_stopButton_clicked();
    void on_startButton_clicked();
    void on_applyButton_clicked();
};

#endif // FORM_TURNONEEIGHTY_H
