TEMPLATE = app
DESTDIR = bin
CONFIG += debug \
    warn_on \
    console
QT += network
UI_DIR = tmp/ui
MOC_DIR = tmp/moc
OBJECTS_DIR = tmp/obj
INCLUDEPATH += ../qextserialport
INCLUDEPATH += ../log4qt
INCLUDEPATH += .
include(../qextserialport/qextserialport.pri)
include(../log4qt/log4qt.pri)
include(../OpenCV.pri)
include(../qwt.pri)
unix:DEFINES += OS_UNIX
win32:DEFINES += OS_WIN32
HEADERS += Framework/healthstatus.h \
    Framework/datarecorder.h \
    Framework/robotmodule.h \
    Framework/mainwindow.h \
    Framework/healthmodel.h \
    Framework/modulesgraph.h \
    Framework/graphview.h \
    Framework/datamodel.h \
    Module_UID/module_uid.h \
    Module_UID/form_uid.h \
    Module_HandControl/server.h \
    Module_HandControl/module_handcontrol.h \
    Module_HandControl/handcontrol_form.h \
    Module_ScanningSonar/sonardatasourcefile.h \
    Module_ScanningSonar/sonardatasourceserial.h \
    Module_ScanningSonar/sonarreturndata.h \
    Module_ScanningSonar/sonardatasource.h \
    Module_ScanningSonar/sonardatarecorder.h \
    Module_ScanningSonar/module_scanningsonar.h \
    Module_Thruster/thruster_form.h \
    Module_Thruster/module_thruster.h \
    Module_ThrusterControlLoop/tcl_form.h \
    Module_ThrusterControlLoop/module_thrustercontrolloop.h \
    Module_PressureSensor/pressure_form.h \
    Module_PressureSensor/module_pressuresensor.h \
    Module_IMU/module_imu.h \
    Module_IMU/imu_form.h \
    Module_Compass/module_compass.h \
    Module_Compass/compass_form.h \
    Module_SonarLocalization/module_sonarlocalization.h \
    Framework/position.h \
    Module_SonarLocalization/form_sonarlocalization.h \
    Framework/robotbehaviour.h \
    Behaviour_PipeFollowing/behaviour_pipefollowing.h \
    Module_ScanningSonar/scanningsonar_form.h \
    Behaviour_PipeFollowing/pipefollowingform.h \
    Framework/hanseapp.h \
    Module_SonarLocalization/sonarechofilter.h \
    Module_SonarLocalization/sonarparticlefilter.h \
    Module_ScanningSonar/sonarswitchcommand.h \
    Module_ScanningSonar/sonardatacsvrecorder.h \
    Module_ScanningSonar/sonardata852recorder.h \
    Module_Navigation/waypointdialog.h \
    Module_Navigation/module_navigation.h \
    Module_Navigation/form_navigation.h \
    Framework/dataloghelper.h \
    Framework/qgraphicsviewextended.h \
    Framework/moduledataview.h \
    Framework/modulehealthview.h \
    Framework/qclosabledockwidget.h \
    Module_Navigation/mapwidget.h \
    MetaBehaviour/metabehaviour.h \
    MetaBehaviour/metabehaviourform.h \
    Behaviour_GoalFollowing/behaviour_goalfollowing.h \
    Behaviour_GoalFollowing/goalfollowingform.h \
    Behaviour_BallFollowing/ballfollowingform.h \
    Behaviour_BallFollowing/behaviour_ballfollowing.h \
    Framework/eventthread.h \
    Module_Webcams/module_webcams.h \
    Module_Webcams/form_webcams.h \
    Behaviour_BallFollowing/blobs/ComponentLabeling.h \
    Behaviour_BallFollowing/blobs/BlobResult.h \
    Behaviour_BallFollowing/blobs/BlobProperties.h \
    Behaviour_BallFollowing/blobs/BlobOperators.h \
    Behaviour_BallFollowing/blobs/BlobLibraryConfiguration.h \
    Behaviour_BallFollowing/blobs/BlobContour.h \
    Behaviour_BallFollowing/blobs/blob.h \
    Behaviour_TurnOneEighty/behaviour_turnoneeighty.h \
    Behaviour_TurnOneEighty/form_turnoneeighty.h \
    Module_Simulation/simulation_form.h \
    Module_Simulation/module_simulation.h \
    Behaviour_TestMT/testmtform.h \
    Behaviour_TestMT/behaviour_testmt.h \
    Behaviour_CompassFollowing/compassfollowingform.h \
    Behaviour_CompassFollowing/behaviour_compassfollowing.h \
    Module_ADC/module_adc.h \
    Module_ADC/adc_form.h \
    Module_EchoSounder/module_echosounder.h \
    Module_EchoSounder/echoswitchcommand.h \
    Module_EchoSounder/echosounderform.h \
    Module_EchoSounder/echoreturndata.h \
    Module_EchoSounder/echodatasourceserial.h \
    Module_EchoSounder/echodatasourcefile.h \
    Module_EchoSounder/echodatasource.h \
    Module_EchoSounder/echodatarecorder.h \
    Module_EchoSounder/echodatacsvrecorder.h \
    Module_EchoSounder/echodata852recorder.h \
    Behaviour_WallFollowing/wallfollowingform.h \
    Behaviour_WallFollowing/behaviour_wallfollowing.h \
    Module_SonarLocalization/SVMClassifier.h \
    Module_SonarLocalization/sonarechofilter.h \
    Module_SonarLocalization/sonarechodata.h \
    Framework/pidcontroller.h \
    Framework/pidwidget.h \
    CommandCenter/commandcenterform.h \
    CommandCenter/commandcenter.h
SOURCES += Framework/robotmodule.cpp \
    Framework/healthstatus.cpp \
    Framework/datarecorder.cpp \
    Framework/mainwindow.cpp \
    Framework/main.cpp \
    Framework/healthmodel.cpp \
    Framework/modulesgraph.cpp \
    Framework/graphview.cpp \
    Framework/datamodel.cpp \
    Module_UID/module_uid.cpp \
    Module_UID/form_uid.cpp \
    Module_HandControl/server.cpp \
    Module_HandControl/module_handcontrol.cpp \
    Module_HandControl/handcontrol_form.cpp \
    Module_ScanningSonar/sonardatasourcefile.cpp \
    Module_ScanningSonar/sonardatasourceserial.cpp \
    Module_ScanningSonar/sonarreturndata.cpp \
    Module_ScanningSonar/sonardatasource.cpp \
    Module_ScanningSonar/sonardatarecorder.cpp \
    Module_ScanningSonar/module_scanningsonar.cpp \
    Module_Thruster/thruster_form.cpp \
    Module_Thruster/module_thruster.cpp \
    Module_ThrusterControlLoop/tcl_form.cpp \
    Module_ThrusterControlLoop/module_thrustercontrolloop.cpp \
    Module_PressureSensor/module_pressuresensor.cpp \
    Module_PressureSensor/pressure_form.cpp \
    Module_IMU/module_imu.cpp \
    Module_IMU/imu_form.cpp \
    Module_Compass/module_compass.cpp \
    Module_Compass/compass_form.cpp \
    Module_SonarLocalization/module_sonarlocalization.cpp \
    Framework/position.cpp \
    Module_SonarLocalization/form_sonarlocalization.cpp \
    Framework/robotbehaviour.cpp \
    Behaviour_PipeFollowing/behaviour_pipefollowing.cpp \
    Module_ScanningSonar/scanningsonar_form.cpp \
    Behaviour_PipeFollowing/pipefollowingform.cpp \
    Framework/hanseapp.cpp \
    Module_SonarLocalization/sonarechofilter.cpp \
    Module_SonarLocalization/sonarparticlefilter.cpp \
    Module_ScanningSonar/sonarswitchcommand.cpp \
    Module_ScanningSonar/sonardatacsvrecorder.cpp \
    Module_ScanningSonar/sonardata852recorder.cpp \
    Module_Navigation/waypointdialog.cpp \
    Module_Navigation/module_navigation.cpp \
    Module_Navigation/form_navigation.cpp \
    Framework/dataloghelper.cpp \
    Framework/qgraphicsviewextended.cpp \
    Framework/moduledataview.cpp \
    Framework/modulehealthview.cpp \
    Framework/qclosabledockwidget.cpp \
    Module_Navigation/mapwidget.cpp \
    MetaBehaviour/metabehaviour.cpp \
    MetaBehaviour/metabehaviourform.cpp \
    Behaviour_GoalFollowing/goalfollowingform.cpp \
    Behaviour_GoalFollowing/behaviour_goalfollowing.cpp \
    Behaviour_BallFollowing/ballfollowingform.cpp \
    Behaviour_BallFollowing/behaviour_ballfollowing.cpp \
    Framework/eventthread.cpp \
    Module_Webcams/module_webcams.cpp \
    Module_Webcams/form_webcams.cpp \
    Behaviour_BallFollowing/blobs/ComponentLabeling.cpp \
    Behaviour_BallFollowing/blobs/BlobResult.cpp \
    Behaviour_BallFollowing/blobs/BlobOperators.cpp \
    Behaviour_BallFollowing/blobs/BlobContour.cpp \
    Behaviour_BallFollowing/blobs/blob.cpp \
    Behaviour_TurnOneEighty/behaviour_turnoneeighty.cpp \
    Behaviour_TurnOneEighty/form_turnoneeighty.cpp \
    Module_Simulation/simulation_form.cpp \
    Module_Simulation/module_simulation.cpp \
    Behaviour_TestMT/testmtform.cpp \
    Behaviour_TestMT/behaviour_testmt.cpp \
    Behaviour_CompassFollowing/compassfollowingform.cpp \
    Behaviour_CompassFollowing/behaviour_compassfollowing.cpp \
    Module_ADC/module_adc.cpp \
    Module_ADC/adc_form.cpp \
    Module_EchoSounder/module_echosounder.cpp \
    Module_EchoSounder/echoswitchcommand.cpp \
    Module_EchoSounder/echosounderform.cpp \
    Module_EchoSounder/echoreturndata.cpp \
    Module_EchoSounder/echodatasourceserial.cpp \
    Module_EchoSounder/echodatasourcefile.cpp \
    Module_EchoSounder/echodatasource.cpp \
    Module_EchoSounder/echodatarecorder.cpp \
    Module_EchoSounder/echodatacsvrecorder.cpp \
    Module_EchoSounder/echodata852recorder.cpp \
    Behaviour_WallFollowing/wallfollowingform.cpp \
    Behaviour_WallFollowing/behaviour_wallfollowing.cpp \
    Module_SonarLocalization/SVMClassifier.cpp \
    Module_SonarLocalization/sonarechodata.cpp \
    Framework/pidcontroller.cpp \
    Framework/pidwidget.cpp \
    CommandCenter/commandcenterform.cpp \
    CommandCenter/commandcenter.cpp
FORMS += Framework/mainwindow.ui \
    Framework/graphview.ui \
    Module_UID/form_uid.ui \
    Module_HandControl/handcontrol_form.ui \
    Module_Thruster/thruster_form.ui \
    Module_ThrusterControlLoop/tcl_form.ui \
    Module_PressureSensor/pressure_form.ui \
    Module_IMU/imu_form.ui \
    Module_Compass/compass_form.ui \
    Module_SonarLocalization/form_sonarlocalization.ui \
    Module_ScanningSonar/scanningsonar_form.ui \
    Behaviour_PipeFollowing/pipefollowingform.ui \
    Module_Navigation/waypointdialog.ui \
    Module_Navigation/form_navigation.ui \
    Framework/moduledataview.ui \
    Framework/modulehealthview.ui \
    Module_Navigation/mapwidget.ui \
    MetaBehaviour/metabehaviourform.ui \
    Behaviour_GoalFollowing/goalfollowingform.ui \
    Behaviour_BallFollowing/ballfollowingform.ui \
    Module_Webcams/form_webcams.ui \
    Behaviour_TurnOneEighty/form_turnoneeighty.ui \
    Module_Simulation/simulation_form.ui \
    Behaviour_TestMT/testmtform.ui \
    Behaviour_CompassFollowing/compassfollowingform.ui \
    Module_ADC/adc_form.ui \
    Module_EchoSounder/echosounderform.ui \
    Behaviour_WallFollowing/wallfollowingform.ui \
    Framework/pidwidget.ui \
    CommandCenter/commandcenterform.ui
OTHER_FILES += bin/log4qt.properties
