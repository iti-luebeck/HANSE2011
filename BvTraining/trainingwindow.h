#ifndef TRAININGWINDOW_H
#define TRAININGWINDOW_H

#include <QMainWindow>

namespace Ui {
    class TrainingWindow;
}

class TrainingWindow : public QMainWindow {
    Q_OBJECT
public:
    TrainingWindow(QWidget *parent = 0);
    ~TrainingWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TrainingWindow *ui;

private slots:
    void on_trainButton_clicked();
};

#endif // TRAININGWINDOW_H
