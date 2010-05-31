#ifndef FORM_SONARLOCALIZATION_H
#define FORM_SONARLOCALIZATION_H

#include <QWidget>

namespace Ui {
    class Form_SonarLocalization;
}

class Form_SonarLocalization : public QWidget {
    Q_OBJECT
public:
    Form_SonarLocalization(QWidget *parent = 0);
    ~Form_SonarLocalization();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_SonarLocalization *ui;
};

#endif // FORM_SONARLOCALIZATION_H
