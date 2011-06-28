#ifndef OBJECTTRACKERWIDGET_H
#define OBJECTTRACKERWIDGET_H

#include <QWidget>
#include <Framework/objecttracker.h>

namespace Ui {
    class ObjectTrackerWidget;
}

class ObjectTrackerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectTrackerWidget(ObjectTracker *tracker, QWidget *parent = 0);
    ~ObjectTrackerWidget();

private:
    void loadTrackerData();
    void storeTrackerData();

private:
    Ui::ObjectTrackerWidget *ui;
    ObjectTracker *tracker;

public slots:
    void trackerUpdated();

private slots:
    void on_applyButton_clicked();
};

#endif // OBJECTTRACKERWIDGET_H
