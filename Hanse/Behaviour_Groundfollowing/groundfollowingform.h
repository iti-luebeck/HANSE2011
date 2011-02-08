#ifndef GROUNDFOLLOWINGFORM_H
#define GROUNDFOLLOWINGFORM_H

#include <QWidget>

namespace Ui {
    class groundfollowingform;
}

class groundfollowingform : public QWidget
{
    Q_OBJECT

public:
    explicit groundfollowingform(QWidget *parent = 0);
    ~groundfollowingform();

private:
    Ui::groundfollowingform *ui;
};

#endif // GROUNDFOLLOWINGFORM_H
