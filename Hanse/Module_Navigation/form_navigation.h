#ifndef FORM_NAVIGATION_H
#define FORM_NAVIGATION_H

#include <QWidget>

namespace Ui {
    class Form_Navigation;
}

class Form_Navigation : public QWidget {
    Q_OBJECT
public:
    Form_Navigation(QWidget *parent = 0);
    ~Form_Navigation();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Navigation *ui;
};

#endif // FORM_NAVIGATION_H
