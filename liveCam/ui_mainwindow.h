/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Tue 29. Jun 15:58:03 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QLineEdit *camIDLineEdit;
    QLabel *frameLabel;
    QPushButton *startButton;
    QPushButton *settingsButton;
    QPushButton *stopButton;
    QPushButton *saveApplybutton;
    QLineEdit *limitLineEdit;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(600, 400);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        camIDLineEdit = new QLineEdit(centralWidget);
        camIDLineEdit->setObjectName(QString::fromUtf8("camIDLineEdit"));
        camIDLineEdit->setGeometry(QRect(520, 10, 31, 20));
        frameLabel = new QLabel(centralWidget);
        frameLabel->setObjectName(QString::fromUtf8("frameLabel"));
        frameLabel->setGeometry(QRect(50, 10, 451, 291));
        frameLabel->setScaledContents(true);
        startButton = new QPushButton(centralWidget);
        startButton->setObjectName(QString::fromUtf8("startButton"));
        startButton->setGeometry(QRect(510, 40, 75, 23));
        settingsButton = new QPushButton(centralWidget);
        settingsButton->setObjectName(QString::fromUtf8("settingsButton"));
        settingsButton->setGeometry(QRect(510, 70, 75, 23));
        stopButton = new QPushButton(centralWidget);
        stopButton->setObjectName(QString::fromUtf8("stopButton"));
        stopButton->setGeometry(QRect(510, 90, 75, 23));
        saveApplybutton = new QPushButton(centralWidget);
        saveApplybutton->setObjectName(QString::fromUtf8("saveApplybutton"));
        saveApplybutton->setGeometry(QRect(510, 110, 75, 23));
        limitLineEdit = new QLineEdit(centralWidget);
        limitLineEdit->setObjectName(QString::fromUtf8("limitLineEdit"));
        limitLineEdit->setGeometry(QRect(560, 10, 31, 20));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 20));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        camIDLineEdit->setText(QApplication::translate("MainWindow", "0", 0, QApplication::UnicodeUTF8));
        frameLabel->setText(QApplication::translate("MainWindow", "videoLabel", 0, QApplication::UnicodeUTF8));
        startButton->setText(QApplication::translate("MainWindow", "START", 0, QApplication::UnicodeUTF8));
        settingsButton->setText(QApplication::translate("MainWindow", "SETTINGS", 0, QApplication::UnicodeUTF8));
        stopButton->setText(QApplication::translate("MainWindow", "STOP", 0, QApplication::UnicodeUTF8));
        saveApplybutton->setText(QApplication::translate("MainWindow", "SAVE APPLY", 0, QApplication::UnicodeUTF8));
        limitLineEdit->setText(QApplication::translate("MainWindow", "-10", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
