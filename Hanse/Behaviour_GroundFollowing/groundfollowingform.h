#ifndef GROUNDFOLLOWINGFORM_H
#define GROUNDFOLLOWINGFORM_H

#include <QWidget>

namespace Ui {
    class GroundFollowingForm;
}

class GroundFollowingForm : public QWidget
{
    Q_OBJECT

public:
    explicit GroundFollowingForm(QWidget *parent = 0);
    ~GroundFollowingForm();

private:
    Ui::GroundFollowingForm *ui;
};

#endif // GROUNDFOLLOWINGFORM_H
