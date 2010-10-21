#ifndef PIPEFOLLOWINGFORM_H
#define PIPEFOLLOWINGFORM_H

#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>

#include <QWidget>

class Behaviour_PipeFollowing;

namespace Ui {
    class PipeFollowingForm;
}

class PipeFollowingForm : public QWidget {
    Q_OBJECT
public:
    PipeFollowingForm(QWidget *parent, Behaviour_PipeFollowing *pipefollowing);
    ~PipeFollowingForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::PipeFollowingForm *ui;
    Behaviour_PipeFollowing *pipefollow;
    QString videoFile;

private slots:
    void on_stopButton_clicked();
    void on_saveApplyButton_clicked();
    void on_startFromVideoFileButton_clicked();
    void on_startPipeFollowingButton_clicked();

public slots:
    void updatePixmap(const QImage &image);

signals:
   void startPipeFollow();
   void stopPipeFollow();
   void newDataPipeFollow(QString key, const QVariant data);
   void newSettingsPipeFollow(QString key, const QVariant value);
   void settingsChanged();

};

#endif // PIPEFOLLOWINGFORM_H
