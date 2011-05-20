#!/bin/bash

set -e

source /opt/ros/diamondback/setup.bash
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:$PWD

#rosmake GamepadTNG

roslaunch GamepadTNG teleop_joy.launch

