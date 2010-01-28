#ifndef MODULE_SCANNINGSONAR_H
#define MODULE_SCANNINGSONAR_H

#include "Module_ScanningSonar_global.h"
#include "robotmodule.h"
#include <QtCore>

class QextSerialPort;
class SonarReturnData;
class SonarDataSource;

class MODULE_SCANNINGSONARSHARED_EXPORT Module_ScanningSonar : public RobotModule {
    Q_OBJECT

    class ThreadedReader : public QThread {
    public:
        ThreadedReader(Module_ScanningSonar* m);

        void pleaseStop();

        void run(void);

    private:
        Module_ScanningSonar* m;
        QTextStream* fileStream;
        bool running; // XXX: volatile


    };

public:
    Module_ScanningSonar(QString id);
    ~Module_ScanningSonar();

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();
    QList<SonarReturnData*> data;

public slots:
    void doNextScan();

public slots:
    void reset();
    void terminate();
    //void enabled(bool value);

signals:
    void newSonarData(void);

private:
    ThreadedReader reader;
    QTimer timer;
    SonarDataSource* source;

};

#endif // MODULE_SCANNINGSONAR_H
