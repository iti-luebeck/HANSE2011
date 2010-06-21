#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>

namespace Ui {
    class MapWidget;
}

class MapWidget : public QWidget {
    Q_OBJECT
public:
    MapWidget(QWidget *parent = 0);
    ~MapWidget();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MapWidget *ui;
};

#endif // MAPWIDGET_H
