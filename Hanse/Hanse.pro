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
include(../gsl.pri)
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
    Module_ScanningSonar/qsonarview.h \
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
    Module_Localization/module_localization.h \
    Module_Navigation/module_navigation.h \
    Module_VisualSLAM/module_visualslam.h \
    Module_SonarLocalization/module_sonarlocalization.h \
    Framework/position.h \
    Module_Localization/form_localization.h \
    Module_Navigation/form_navigation.h \
    Module_SonarLocalization/form_sonarlocalization.h \
    Framework/robotbehaviour.h \
    Behaviour_PipeFollowing/behaviour_pipefollowing.h \
    Module_ScanningSonar/scanningsonar_form.h \
    Module_VisualSLAM/helpers.h \
    Module_VisualSLAM/odometry/odometry.h \
    Module_VisualSLAM/feature/feature.h \
    Module_VisualSLAM/feature/surf/utils.h \
    Module_VisualSLAM/feature/surf/surflib.h \
    Module_VisualSLAM/feature/surf/surf.h \
    Module_VisualSLAM/feature/surf/responselayer.h \
    Module_VisualSLAM/feature/surf/kmeans.h \
    Module_VisualSLAM/feature/surf/ipoint.h \
    Module_VisualSLAM/feature/surf/integral.h \
    Module_VisualSLAM/feature/surf/fasthessian.h \
    Module_VisualSLAM/feature/fast/cvfast.h \
    Module_VisualSLAM/capture/stereocapture.h \
    Module_VisualSLAM/slam/quaternion.h \
    Module_VisualSLAM/slam/particle.h \
    Module_VisualSLAM/slam/naiveslam.h \
    Module_VisualSLAM/slam/landmark.h \
    Module_VisualSLAM/form_visualslam.h \
    Behaviour_PipeFollowing/pipefollowingform.h \
    Framework/hanseapp.h \
    Module_Navigation/waypointdialog.h
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
    Module_ScanningSonar/qsonarview.cpp \
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
    Module_Localization/module_localization.cpp \
    Module_Navigation/module_navigation.cpp \
    Module_VisualSLAM/module_visualslam.cpp \
    Module_SonarLocalization/module_sonarlocalization.cpp \
    Framework/position.cpp \
    Module_Localization/form_localization.cpp \
    Module_Navigation/form_navigation.cpp \
    Module_SonarLocalization/form_sonarlocalization.cpp \
    Framework/robotbehaviour.cpp \
    Behaviour_PipeFollowing/behaviour_pipefollowing.cpp \
    Module_ScanningSonar/scanningsonar_form.cpp \
    Module_VisualSLAM/odometry/odometry.cpp \
    Module_VisualSLAM/feature/feature.cpp \
    Module_VisualSLAM/feature/surf/utils.cpp \
    Module_VisualSLAM/feature/surf/surf.cpp \
    Module_VisualSLAM/feature/surf/ipoint.cpp \
    Module_VisualSLAM/feature/surf/integral.cpp \
    Module_VisualSLAM/feature/surf/fasthessian.cpp \
    Module_VisualSLAM/feature/fast/cvfast.cpp \
    Module_VisualSLAM/feature/fast/cv_fast_12.cc \
    Module_VisualSLAM/feature/fast/cv_fast_11.cc \
    Module_VisualSLAM/feature/fast/cv_fast_10.cc \
    Module_VisualSLAM/feature/fast/cv_fast_9.cc \
    Module_VisualSLAM/capture/stereocapture.cpp \
    Module_VisualSLAM/slam/quaternion.cpp \
    Module_VisualSLAM/slam/particle.cpp \
    Module_VisualSLAM/slam/naiveslam.cpp \
    Module_VisualSLAM/slam/landmark.cpp \
    Module_VisualSLAM/form_visualslam.cpp \
    Behaviour_PipeFollowing/pipefollowingform.cpp \
    Framework/hanseapp.cpp \
    Module_Navigation/waypointdialog.cpp
FORMS += Framework/mainwindow.ui \
    Framework/graphview.ui \
    Module_UID/form_uid.ui \
    Module_HandControl/handcontrol_form.ui \
    Module_Thruster/thruster_form.ui \
    Module_ThrusterControlLoop/tcl_form.ui \
    Module_PressureSensor/pressure_form.ui \
    Module_IMU/imu_form.ui \
    Module_Compass/compass_form.ui \
    Module_Localization/form_localization.ui \
    Module_Navigation/form_navigation.ui \
    Module_SonarLocalization/form_sonarlocalization.ui \
    Module_ScanningSonar/scanningsonar_form.ui \
    Module_VisualSLAM/form_visualslam.ui \
    Behaviour_PipeFollowing/pipefollowingform.ui \
    Module_Navigation/waypointdialog.ui
