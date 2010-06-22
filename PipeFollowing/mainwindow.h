#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "PipeFollow.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    PipeFollow pipeFollow;
    QString videoFile;
private slots:
    void on_loadVideoButton_clicked();
    void on_detLineButton_clicked();
    void on_loadPictureButton_clicked();


};

#endif // MAINWINDOW_H
