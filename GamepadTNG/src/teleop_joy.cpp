#include <QtCore>

#include <ros/ros.h>
#include <joy/Joy.h>
#include <geometry_msgs/Twist.h>
#include <dynamic_reconfigure/server.h>

#include <GamepadTNG/NodeConfig.h>
#include "client.h"

class TeleopHanse
{
public:
  TeleopHanse();

private:

  void joyCallback(const joy::Joy::ConstPtr& joy);
  void timerCallback(const ros::TimerEvent &e);
  
  void dyn_reconfigure_callback(GamepadTNG::NodeConfig &config, uint32_t level);

  Client client;

  ros::NodeHandle node;

  ros::Timer control_loop_timer;

  ros::Publisher pub_cmd_vel;
  ros::Subscriber sub_joy;

  double value_linear;
  double value_angular;
  double value_depth;

  bool emergency_stop;
  bool enable_handcontrol;

  bool config_set;

  GamepadTNG::NodeConfig config;
  
  /** \brief dynamic_reconfigure interface */
  dynamic_reconfigure::Server<GamepadTNG::NodeConfig> dyn_reconfigure_srv;

  /** \brief dynamic_reconfigure call back */
  dynamic_reconfigure::Server<GamepadTNG::NodeConfig>::CallbackType dyn_reconfigure_cb;

};

TeleopHanse::TeleopHanse() {
  value_linear = 0;
  value_angular = 0;
  value_depth = 0;
  emergency_stop = false;
  enable_handcontrol = false;

  pub_cmd_vel = node.advertise<geometry_msgs::Twist>("/cmd_vel", 1);
  sub_joy = node.subscribe<joy::Joy>("joy", 10, &TeleopHanse::joyCallback, this);

  // will be set to actual value once config is loaded
  control_loop_timer = node.createTimer(ros::Duration(1), &TeleopHanse::timerCallback, this);

  dyn_reconfigure_cb = boost::bind(&TeleopHanse::dyn_reconfigure_callback, this, _1, _2);
  dyn_reconfigure_srv.setCallback(dyn_reconfigure_cb);
  // from this point on we can assume a valid config

  ROS_INFO("teleop_joy started");


}

void TeleopHanse::dyn_reconfigure_callback(GamepadTNG::NodeConfig &config, uint32_t level)
{
    ROS_INFO("got new parameters, level=%d", level);

    this->config = config;

    client.setConfig(QString(config.server_hosts.c_str()), config.server_port);

    control_loop_timer.setPeriod(ros::Duration(1.0/config.frequency));
}

void TeleopHanse::timerCallback(const ros::TimerEvent &e)
{
    // publish data on topic. no use for this, yet
    geometry_msgs::Twist vel;
    vel.angular.z = value_angular;
    vel.linear.x = value_linear;
    vel.linear.z = value_depth;
    pub_cmd_vel.publish(vel);

    client.sendMessage(value_linear, value_angular, value_depth, emergency_stop, enable_handcontrol);
//    client.sendMessage(0, 100, 200, emergency_stop, enable_handcontrol);

    if (emergency_stop)
        ROS_INFO("Pressing emergency_stop button");
    if (enable_handcontrol)
        ROS_INFO("Pressing enable_handcontrol button");
}

void TeleopHanse::joyCallback(const joy::Joy::ConstPtr& joy)
{
    if (fabs(joy->axes[config.axis_angular]) < config.joy_gate)
        value_angular = 0;
    else
        value_angular = config.scale_angular * joy->axes[config.axis_angular];

    if (fabs(joy->axes[config.axis_linear]) < config.joy_gate)
        value_linear = 0;
    else
        value_linear = config.scale_linear * joy->axes[config.axis_linear];

    value_depth = config.scale_depth * (joy->axes[config.axis_depth]+1);

    emergency_stop = joy->buttons[config.button_emegency_stop];
    enable_handcontrol = joy->buttons[config.button_hand_control];
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "teleop_hanse");

  ros::start();

  TeleopHanse teleop_hanse;

  ros::spin();

}

