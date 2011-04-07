/********************************************************************************
** Form generated from reading UI file 'sltrainingui.ui'
**
** Created: Thu Apr 7 17:29:16 2011
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SLTRAININGUI_H
#define UI_SLTRAININGUI_H

#include <Framework/qgraphicsviewextended.h>
#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QFormLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SLTrainingUI
{
public:
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QWidget *tab;
    QWidget *verticalLayoutWidget;
    QFormLayout *formLayout;
    QPushButton *loadSonarFile;
    QPushButton *selectSamples;
    QLabel *selSampleLabel;
    QSlider *selSampleWidthSlider;
    QPushButton *loadSVM;
    QSpinBox *maxSamples;
    QPushButton *saveSVM;
    QPushButton *trainSVM;
    QPushButton *testSVM;
    QCheckBox *classify;
    QPushButton *pushButton;
    QDateTimeEdit *startTime;
    QWidget *tab_2;
    QLineEdit *gain;
    QCheckBox *checkBox_3;
    QLineEdit *range;
    QLineEdit *heading;
    QDateTimeEdit *time;
    QWidget *tab_3;
    QWidget *formLayoutWidget;
    QFormLayout *formLayout_2;
    QLabel *Gausslabel;
    QLineEdit *gaussfactor;
    QLabel *VarianceThreshlabel;
    QLineEdit *varianceThresh;
    QLabel *Wallwindowlabel;
    QLineEdit *wallwindowsize;
    QLabel *LargePeakTHlabel_2;
    QLineEdit *largePeakTH;
    QLabel *MeanBehindLabel;
    QLineEdit *meanBehindTH;
    QGraphicsViewExtended *graphicsView_2;

    void setupUi(QMainWindow *SLTrainingUI)
    {
        if (SLTrainingUI->objectName().isEmpty())
            SLTrainingUI->setObjectName(QString::fromUtf8("SLTrainingUI"));
        SLTrainingUI->resize(989, 705);
        centralWidget = new QWidget(SLTrainingUI);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(40, 20, 351, 541));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayoutWidget = new QWidget(tab);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 10, 341, 401));
        formLayout = new QFormLayout(verticalLayoutWidget);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        formLayout->setContentsMargins(0, 0, 0, 0);
        loadSonarFile = new QPushButton(verticalLayoutWidget);
        loadSonarFile->setObjectName(QString::fromUtf8("loadSonarFile"));

        formLayout->setWidget(0, QFormLayout::LabelRole, loadSonarFile);

        selectSamples = new QPushButton(verticalLayoutWidget);
        selectSamples->setObjectName(QString::fromUtf8("selectSamples"));

        formLayout->setWidget(1, QFormLayout::LabelRole, selectSamples);

        selSampleLabel = new QLabel(verticalLayoutWidget);
        selSampleLabel->setObjectName(QString::fromUtf8("selSampleLabel"));

        formLayout->setWidget(2, QFormLayout::LabelRole, selSampleLabel);

        selSampleWidthSlider = new QSlider(verticalLayoutWidget);
        selSampleWidthSlider->setObjectName(QString::fromUtf8("selSampleWidthSlider"));
        selSampleWidthSlider->setMinimum(1);
        selSampleWidthSlider->setMaximum(6);
        selSampleWidthSlider->setOrientation(Qt::Horizontal);

        formLayout->setWidget(2, QFormLayout::FieldRole, selSampleWidthSlider);

        loadSVM = new QPushButton(verticalLayoutWidget);
        loadSVM->setObjectName(QString::fromUtf8("loadSVM"));

        formLayout->setWidget(4, QFormLayout::LabelRole, loadSVM);

        maxSamples = new QSpinBox(verticalLayoutWidget);
        maxSamples->setObjectName(QString::fromUtf8("maxSamples"));
        maxSamples->setMinimum(500);
        maxSamples->setMaximum(300000);
        maxSamples->setValue(10000);

        formLayout->setWidget(4, QFormLayout::FieldRole, maxSamples);

        saveSVM = new QPushButton(verticalLayoutWidget);
        saveSVM->setObjectName(QString::fromUtf8("saveSVM"));

        formLayout->setWidget(5, QFormLayout::LabelRole, saveSVM);

        trainSVM = new QPushButton(verticalLayoutWidget);
        trainSVM->setObjectName(QString::fromUtf8("trainSVM"));

        formLayout->setWidget(6, QFormLayout::LabelRole, trainSVM);

        testSVM = new QPushButton(verticalLayoutWidget);
        testSVM->setObjectName(QString::fromUtf8("testSVM"));

        formLayout->setWidget(7, QFormLayout::LabelRole, testSVM);

        classify = new QCheckBox(verticalLayoutWidget);
        classify->setObjectName(QString::fromUtf8("classify"));
        classify->setChecked(true);

        formLayout->setWidget(8, QFormLayout::LabelRole, classify);

        pushButton = new QPushButton(verticalLayoutWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        formLayout->setWidget(9, QFormLayout::LabelRole, pushButton);

        startTime = new QDateTimeEdit(verticalLayoutWidget);
        startTime->setObjectName(QString::fromUtf8("startTime"));
        startTime->setDate(QDate(2010, 8, 27));
        startTime->setTime(QTime(13, 52, 0));

        formLayout->setWidget(5, QFormLayout::FieldRole, startTime);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gain = new QLineEdit(tab_2);
        gain->setObjectName(QString::fromUtf8("gain"));
        gain->setGeometry(QRect(10, 90, 113, 27));
        checkBox_3 = new QCheckBox(tab_2);
        checkBox_3->setObjectName(QString::fromUtf8("checkBox_3"));
        checkBox_3->setGeometry(QRect(10, 180, 98, 22));
        checkBox_3->setChecked(true);
        range = new QLineEdit(tab_2);
        range->setObjectName(QString::fromUtf8("range"));
        range->setGeometry(QRect(10, 120, 113, 27));
        heading = new QLineEdit(tab_2);
        heading->setObjectName(QString::fromUtf8("heading"));
        heading->setGeometry(QRect(10, 150, 113, 27));
        time = new QDateTimeEdit(tab_2);
        time->setObjectName(QString::fromUtf8("time"));
        time->setGeometry(QRect(10, 210, 194, 27));
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        formLayoutWidget = new QWidget(tab_3);
        formLayoutWidget->setObjectName(QString::fromUtf8("formLayoutWidget"));
        formLayoutWidget->setGeometry(QRect(0, 30, 191, 161));
        formLayout_2 = new QFormLayout(formLayoutWidget);
        formLayout_2->setSpacing(6);
        formLayout_2->setContentsMargins(11, 11, 11, 11);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        formLayout_2->setContentsMargins(0, 0, 0, 0);
        Gausslabel = new QLabel(formLayoutWidget);
        Gausslabel->setObjectName(QString::fromUtf8("Gausslabel"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, Gausslabel);

        gaussfactor = new QLineEdit(formLayoutWidget);
        gaussfactor->setObjectName(QString::fromUtf8("gaussfactor"));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, gaussfactor);

        VarianceThreshlabel = new QLabel(formLayoutWidget);
        VarianceThreshlabel->setObjectName(QString::fromUtf8("VarianceThreshlabel"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, VarianceThreshlabel);

        varianceThresh = new QLineEdit(formLayoutWidget);
        varianceThresh->setObjectName(QString::fromUtf8("varianceThresh"));

        formLayout_2->setWidget(1, QFormLayout::FieldRole, varianceThresh);

        Wallwindowlabel = new QLabel(formLayoutWidget);
        Wallwindowlabel->setObjectName(QString::fromUtf8("Wallwindowlabel"));

        formLayout_2->setWidget(2, QFormLayout::LabelRole, Wallwindowlabel);

        wallwindowsize = new QLineEdit(formLayoutWidget);
        wallwindowsize->setObjectName(QString::fromUtf8("wallwindowsize"));

        formLayout_2->setWidget(2, QFormLayout::FieldRole, wallwindowsize);

        LargePeakTHlabel_2 = new QLabel(formLayoutWidget);
        LargePeakTHlabel_2->setObjectName(QString::fromUtf8("LargePeakTHlabel_2"));

        formLayout_2->setWidget(3, QFormLayout::LabelRole, LargePeakTHlabel_2);

        largePeakTH = new QLineEdit(formLayoutWidget);
        largePeakTH->setObjectName(QString::fromUtf8("largePeakTH"));

        formLayout_2->setWidget(3, QFormLayout::FieldRole, largePeakTH);

        MeanBehindLabel = new QLabel(formLayoutWidget);
        MeanBehindLabel->setObjectName(QString::fromUtf8("MeanBehindLabel"));

        formLayout_2->setWidget(4, QFormLayout::LabelRole, MeanBehindLabel);

        meanBehindTH = new QLineEdit(formLayoutWidget);
        meanBehindTH->setObjectName(QString::fromUtf8("meanBehindTH"));

        formLayout_2->setWidget(4, QFormLayout::FieldRole, meanBehindTH);

        tabWidget->addTab(tab_3, QString());
        graphicsView_2 = new QGraphicsViewExtended(centralWidget);
        graphicsView_2->setObjectName(QString::fromUtf8("graphicsView_2"));
        graphicsView_2->setGeometry(QRect(420, 40, 521, 621));
        SLTrainingUI->setCentralWidget(centralWidget);

        retranslateUi(SLTrainingUI);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SLTrainingUI);
    } // setupUi

    void retranslateUi(QMainWindow *SLTrainingUI)
    {
        SLTrainingUI->setWindowTitle(QApplication::translate("SLTrainingUI", "MainWindow", 0, QApplication::UnicodeUTF8));
        loadSonarFile->setText(QApplication::translate("SLTrainingUI", "Load SonarData", 0, QApplication::UnicodeUTF8));
        selectSamples->setText(QApplication::translate("SLTrainingUI", "Select Samples", 0, QApplication::UnicodeUTF8));
        selSampleLabel->setText(QApplication::translate("SLTrainingUI", "Selected Sample Width ():", 0, QApplication::UnicodeUTF8));
        loadSVM->setText(QApplication::translate("SLTrainingUI", "Load SVM", 0, QApplication::UnicodeUTF8));
        saveSVM->setText(QApplication::translate("SLTrainingUI", "Save SVM", 0, QApplication::UnicodeUTF8));
        trainSVM->setText(QApplication::translate("SLTrainingUI", "Train SVM", 0, QApplication::UnicodeUTF8));
        testSVM->setText(QApplication::translate("SLTrainingUI", "Test SVM", 0, QApplication::UnicodeUTF8));
        classify->setText(QApplication::translate("SLTrainingUI", "classify", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("SLTrainingUI", "PushButton", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("SLTrainingUI", "Main", 0, QApplication::UnicodeUTF8));
        checkBox_3->setText(QApplication::translate("SLTrainingUI", "UpdateUI", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("SLTrainingUI", "UI", 0, QApplication::UnicodeUTF8));
        Gausslabel->setText(QApplication::translate("SLTrainingUI", "Gauss Factor", 0, QApplication::UnicodeUTF8));
        gaussfactor->setText(QApplication::translate("SLTrainingUI", "0.7870", 0, QApplication::UnicodeUTF8));
        VarianceThreshlabel->setText(QApplication::translate("SLTrainingUI", "Variance Thresh", 0, QApplication::UnicodeUTF8));
        varianceThresh->setText(QApplication::translate("SLTrainingUI", "0.04", 0, QApplication::UnicodeUTF8));
        Wallwindowlabel->setText(QApplication::translate("SLTrainingUI", "WallWindow Size", 0, QApplication::UnicodeUTF8));
        wallwindowsize->setText(QApplication::translate("SLTrainingUI", "3", 0, QApplication::UnicodeUTF8));
        LargePeakTHlabel_2->setText(QApplication::translate("SLTrainingUI", "Large Peak TH", 0, QApplication::UnicodeUTF8));
        largePeakTH->setText(QApplication::translate("SLTrainingUI", "0.5", 0, QApplication::UnicodeUTF8));
        MeanBehindLabel->setText(QApplication::translate("SLTrainingUI", "Mean Behind TH", 0, QApplication::UnicodeUTF8));
        meanBehindTH->setText(QApplication::translate("SLTrainingUI", "1", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("SLTrainingUI", "Wall Conf", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SLTrainingUI: public Ui_SLTrainingUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SLTRAININGUI_H
