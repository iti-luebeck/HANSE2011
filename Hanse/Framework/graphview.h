#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>

namespace Ui {
    class GraphView;
}

class GraphView : public QWidget {
    Q_OBJECT
public:
    GraphView(QWidget *parent = 0);
    ~GraphView();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::GraphView *ui;
};

#endif // GRAPHVIEW_H
