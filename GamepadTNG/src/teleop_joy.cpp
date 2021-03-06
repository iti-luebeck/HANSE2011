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

  bool button_up;
  bool button_down;

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
  button_up = false;
  button_down = false;

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
    if (!config.use_throttle && button_down) {
        value_depth += config.depth_delta;
        button_down = 0;
    }
    if (!config.use_throttle && button_up) {
        value_depth -= config.depth_delta;
        button_up = 0;
    }
    
    if (value_depth < 0)
	value_depth = 0;

    // publish data on topic. no use for this, yet
    geometry_msgs::Twist vel;
    vel.angular.z = value_angular;
    vel.linear.x = value_linear;
    vel.linear.z = value_depth;

    ROS_INFO("Current target depth: %f cm - FF %f - ANG %f", value_depth, value_linear, value_angular);
    std::cout << "Forward " << value_linear << " Angular " << value_angular << std::endl;

    pub_cmd_vel.publish(vel);

    client.sendMessage(value_linear, value_angular, value_depth, emergency_stop, enable_handcontrol);

    if (emergency_stop) {
        ROS_INFO("Pressed emergency_stop button");
        emergency_stop = 0;
	value_depth = 0;
	ROS_INFO("value_depth set to 0");
    }
    if (enable_handcontrol) {
        ROS_INFO("Pressed enable_handcontrol button");
        enable_handcontrol = 0;
    }
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

    if (config.use_throttle)
	value_depth = config.scale_depth * (joy->axes[config.axis_depth]+1);

    if (joy->buttons[config.button_emegency_stop])
	emergency_stop = 1;

    if (joy->buttons[config.button_hand_control])
        enable_handcontrol = 1;

    if (joy->buttons[config.button_down])
	button_down = 1;

    if (joy->buttons[config.button_up])
        button_up = 1;
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "teleop_hanse");

  ros::start();

  TeleopHanse teleop_hanse;

  ros::spin();

}

