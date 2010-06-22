/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Thu 3. Jun 21:14:23 2010
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
#include <QtGui/QCheckBox>
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
    QPushButton *loadVideoButton;
    QPushButton *detLineButton;
    QCheckBox *debugCheckBox;
    QLineEdit *segmentationThreshLineEdit;
    QLabel *segmentationThreshLAbel;
    QPushButton *loadPictureButton;
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
        loadVideoButton = new QPushButton(centralWidget);
        loadVideoButton->setObjectName(QString::fromUtf8("loadVideoButton"));
        loadVideoButton->setGeometry(QRect(40, 50, 75, 23));
        detLineButton = new QPushButton(centralWidget);
        detLineButton->setObjectName(QString::fromUtf8("detLineButton"));
        detLineButton->setGeometry(QRect(40, 80, 75, 23));
        debugCheckBox = new QCheckBox(centralWidget);
        debugCheckBox->setObjectName(QString::fromUtf8("debugCheckBox"));
        debugCheckBox->setGeometry(QRect(170, 50, 70, 17));
        segmentationThreshLineEdit = new QLineEdit(centralWidget);
        segmentationThreshLineEdit->setObjectName(QString::fromUtf8("segmentationThreshLineEdit"));
        segmentationThreshLineEdit->setGeometry(QRect(170, 90, 113, 20));
        segmentationThreshLAbel = new QLabel(centralWidget);
        segmentationThreshLAbel->setObjectName(QString::fromUtf8("segmentationThreshLAbel"));
        segmentationThreshLAbel->setGeometry(QRect(170, 70, 131, 16));
        loadPictureButton = new QPushButton(centralWidget);
        loadPictureButton->setObjectName(QString::fromUtf8("loadPictureButton"));
        loadPictureButton->setGeometry(QRect(30, 180, 75, 23));
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
        loadVideoButton->setText(QApplication::translate("MainWindow", "load", 0, QApplication::UnicodeUTF8));
        detLineButton->setText(QApplication::translate("MainWindow", "linie suchen", 0, QApplication::UnicodeUTF8));
        debugCheckBox->setText(QApplication::translate("MainWindow", "Debug", 0, QApplication::UnicodeUTF8));
        segmentationThreshLineEdit->setText(QApplication::translate("MainWindow", "200", 0, QApplication::UnicodeUTF8));
        segmentationThreshLAbel->setText(QApplication::translate("MainWindow", "Segmantation Threshold", 0, QApplication::UnicodeUTF8));
        loadPictureButton->setText(QApplication::translate("MainWindow", "loadPic", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
