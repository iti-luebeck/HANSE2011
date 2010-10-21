#ifndef TestMTFORM_H
#define TestMTFORM_H

#include <Behaviour_TestMT/behaviour_testmt.h>

#include <QWidget>

class Behaviour_TestMT;

namespace Ui {
    class TestMTForm;
}

class TestMTForm : public QWidget {
    Q_OBJECT
public:
    TestMTForm(QWidget *parent, Behaviour_TestMT *TestMT);
    ~TestMTForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TestMTForm *ui;
    Behaviour_TestMT *testmt;

private slots:

};

#endif // TestMTFORM_H
