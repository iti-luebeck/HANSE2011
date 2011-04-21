TEMPLATE = app
DESTDIR = bin
CONFIG += debug \
    warn_on \
    console
QT += network
UI_DIR = tmp/ui
MOC_DIR = tmp/moc
OBJECTS_DIR = tmp/obj
INCLUDEPATH += .

include(../log4qt/log4qt.pri)
include(../OpenCV.pri)
include(../qwt.pri)
include(../qextserialport/qextserialport.pri)

include(Module_XsensMTi/Module_XsensMTi.pri)
include(Module_ADC/Module_ADC.pri)
include(Module_Compass/Module_Compass.pri)
include(Module_EchoSounder/Module_EchoSounder.pri)
include(Module_HandControl/Module_HandControl.pri)
include(Module_IMU/Module_IMU.pri)
include(Module_Navigation/Module_Navigation.pri)
include(Module_PressureSensor/Module_PressureSensor.pri)
include(Framework/Framework.pri)
include(CommandCenter/CommandCenter.pri)

HEADERS += Module_UID/module_uid.h \
    Module_UID/form_uid.h \
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
    Module_SonarLocalization/module_sonarlocalization.h \
    Module_SonarLocalization/form_sonarlocalization.h \
    Behaviour_PipeFollowing/behaviour_pipefollowing.h \
    Module_ScanningSonar/scanningsonar_form.h \
    Behaviour_PipeFollowing/pipefollowingform.h \
    Module_SonarLocalization/sonarechofilter.h \
    Module_SonarLocalization/sonarparticlefilter.h \
    Module_ScanningSonar/sonarswitchcommand.h \
    Module_ScanningSonar/sonardatacsvrecorder.h \
    Module_ScanningSonar/sonardata852recorder.h \
    MetaBehaviour/metabehaviour.h \
    MetaBehaviour/metabehaviourform.h \
    Behaviour_GoalFollowing/behaviour_goalfollowing.h \
    Behaviour_GoalFollowing/goalfollowingform.h \
    Behaviour_BallFollowing/ballfollowingform.h \
    Behaviour_BallFollowing/behaviour_ballfollowing.h \
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
    Behaviour_WallFollowing/wallfollowingform.h \
    Behaviour_WallFollowing/behaviour_wallfollowing.h \
    Module_SonarLocalization/sonarechodata.h \
    Module_SLTraining/sltrainingui.h \
    Module_SLTraining/SVMClassifier.h \
    Module_SLTraining/ui_sltrainingui.h \
    Module_SLTraining/sonardatasourcefilereader.h
SOURCES += Module_UID/module_uid.cpp \
    Module_UID/form_uid.cpp \
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
    Module_SonarLocalization/module_sonarlocalization.cpp \
    Module_SonarLocalization/form_sonarlocalization.cpp \
    Behaviour_PipeFollowing/behaviour_pipefollowing.cpp \
    Module_ScanningSonar/scanningsonar_form.cpp \
    Behaviour_PipeFollowing/pipefollowingform.cpp \
    Module_SonarLocalization/sonarechofilter.cpp \
    Module_SonarLocalization/sonarparticlefilter.cpp \
    Module_ScanningSonar/sonarswitchcommand.cpp \
    Module_ScanningSonar/sonardatacsvrecorder.cpp \
    Module_ScanningSonar/sonardata852recorder.cpp \
    MetaBehaviour/metabehaviour.cpp \
    MetaBehaviour/metabehaviourform.cpp \
    Behaviour_GoalFollowing/goalfollowingform.cpp \
    Behaviour_GoalFollowing/behaviour_goalfollowing.cpp \
    Behaviour_BallFollowing/ballfollowingform.cpp \
    Behaviour_BallFollowing/behaviour_ballfollowing.cpp \
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
    Behaviour_WallFollowing/wallfollowingform.cpp \
    Behaviour_WallFollowing/behaviour_wallfollowing.cpp \
    Module_SonarLocalization/sonarechodata.cpp \
    Module_SLTraining/sltrainingui.cpp \
    Module_SLTraining/SVMClassifier.cpp \
    Module_SLTraining/sonardatasourcefilereader.cpp
FORMS +=     Module_UID/form_uid.ui \
    Module_Thruster/thruster_form.ui \
    Module_ThrusterControlLoop/tcl_form.ui \
    Module_SonarLocalization/form_sonarlocalization.ui \
    Module_ScanningSonar/scanningsonar_form.ui \
    Behaviour_PipeFollowing/pipefollowingform.ui \
    MetaBehaviour/metabehaviourform.ui \
    Behaviour_GoalFollowing/goalfollowingform.ui \
    Behaviour_BallFollowing/ballfollowingform.ui \
    Module_Webcams/form_webcams.ui \
    Behaviour_TurnOneEighty/form_turnoneeighty.ui \
    Module_Simulation/simulation_form.ui \
    Behaviour_TestMT/testmtform.ui \
    Behaviour_CompassFollowing/compassfollowingform.ui \
    Behaviour_WallFollowing/wallfollowingform.ui \
    Module_SLTraining/sltrainingui.ui
OTHER_FILES += bin/log4qt.properties
