#ifndef MODULE_SONARLOCALIZATION_H
#define MODULE_SONARLOCALIZATION_H

#include <Framework/robotmodule.h>
#include <Framework/position.h>
#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>

class Module_ScanningSonar;

class Module_SonarLocalization : public RobotModule {
    Q_OBJECT

    friend class Form_SonarLocalization;

public:
    Module_SonarLocalization(QString id, Module_ScanningSonar* sonar);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    /**
      * Returns the current position and orientation of the robot.
      *
      */
    Position getLocalization();

    /**
      * How long ago has the position been updated.
      * (returns unix time: ellapsed seconds since 1070)
      */
    QDateTime getLastRefreshTime();

    /**
      * Returns the confidence, by which the Position returned by
      * getLocalization() should be meassured.
      *
      * Range: 0-1 (0==no confidence; 1==full confidence)
      */
    float getLocalizationConfidence();

    /**
      * Returns true if the robot is lost, i.e. the confidence is
      * below a certain threshold.
      */
    bool isLocalizationLost();

public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);

    void newLocalizationEstimate();
    void lostLocalization();

private slots:
    void newSonarData(SonarReturnData data);

private:

    const static int N = 250;

    const static float stdDevInWindowTH = 0.04;
    const static float meanBehindTH = 1;

    QMap<QDateTime,QVector<double> > echoHistory;
    QMap<QDateTime, int > kHistory;
    QMap<QDateTime, QVector<double> > threshHistory;
    QMap<QDateTime, QVector<double> > varHistory;
    QVector<int> K_history;

    Module_ScanningSonar* sonar;

    QByteArray filterEcho(SonarReturnData data,QByteArray echo);

    int findWall(SonarReturnData data,const QByteArray echo);

    cv::Mat byteArray2Mat(QByteArray array);


};

#endif // MODULE_SONARLOCALIZATION_H
