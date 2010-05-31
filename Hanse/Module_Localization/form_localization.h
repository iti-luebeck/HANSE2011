#ifndef FORM_LOCALIZATION_H
#define FORM_LOCALIZATION_H

#include <QWidget>

namespace Ui {
    class Form_Localization;
}

class Form_Localization : public QWidget {
    Q_OBJECT
public:
    Form_Localization(QWidget *parent = 0);
    ~Form_Localization();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Localization *ui;
};

#endif // FORM_LOCALIZATION_H
