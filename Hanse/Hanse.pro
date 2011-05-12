
# this is a Qt GUI app
TEMPLATE = app

# put resulting binary there
DESTDIR = bin

# enable console to aid debugging
CONFIG += debug \
    warn_on \
    console

# handcontrol needs this
QT += network

# explicit directories for tmp data
UI_DIR = tmp/ui
MOC_DIR = tmp/moc
OBJECTS_DIR = tmp/obj

# so they show up the the project tree, for convenience
OTHER_FILES += bin/log4qt.properties bin/ITI/Hanse.ini \
    TaskHandControl/TaskHandControl.pri \
    Behaviour_XsensFollowing/Behaviour_CompassFollowing.pri

# QMAKE_CXXFLAGS += -O2 -funroll-loops -msse2 -mfpmath=sse

# external libraries
include(../log4qt/log4qt.pri)
include(../OpenCV.pri)
include(../qwt.pri)
include(../qextserialport/qextserialport.pri)

# core stuff
include(Framework/Framework.pri)
include(CommandCenter/CommandCenter.pri)

# all modules
include(Module_Simulation/Module_Simulation.pri)
include(Module_SLTraining/Module_SLTraining.pri)
include(Module_SonarLocalization/Module_SonarLocalization.pri)
include(Module_Thruster/Module_Thruster.pri)
include(Module_ThrusterControlLoop/Module_ThrusterControlLoop.pri)
include(Module_UID/Module_UID.pri)
include(Module_Webcams/Module_Webcams.pri)
include(Module_XsensMTi/Module_XsensMTi.pri)
include(Module_ADC/Module_ADC.pri)
include(Module_Compass/Module_Compass.pri)
include(Module_ScanningSonar/Module_ScanningSonar.pri)
include(Module_EchoSounder/Module_EchoSounder.pri)
include(Module_HandControl/Module_HandControl.pri)
include(Module_IMU/Module_IMU.pri)
include(Module_Navigation/Module_Navigation.pri)
include(Module_PressureSensor/Module_PressureSensor.pri)

# behaviours
include(Behaviour_BallFollowing/Behaviour_BallFollowing.pri)
include(Behaviour_CompassFollowing/Behaviour_CompassFollowing.pri)
include(Behaviour_GoalFollowing/Behaviour_GoalFollowing.pri)
include(Behaviour_PipeFollowing/Behaviour_PipeFollowing.pri)
include(Behaviour_TestMT/Behaviour_TestMT.pri)
include(Behaviour_TurnOneEighty/Behaviour_TurnOneEighty.pri)
include(Behaviour_WallFollowing/Behaviour_WallFollowing.pri)
include(Behaviour_XsensFollowing/Behaviour_XsensFollowing.pri)

# tasks
include(TaskHandControl/TaskHandControl.pri)
include(TaskWallNavigation/TaskWallNavigation.pri)
