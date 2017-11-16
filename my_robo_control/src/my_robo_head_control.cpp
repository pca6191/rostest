#include <ros/ros.h>
#include <ros/package.h>
#include <angles/angles.h>
#include <my_robo_control/my_robo_hw.h>
#include <iostream> // for debug
#include <controller_manager/controller_manager.h>
#include <std_msgs/String.h>

// 首へ指令値を送るpublisher
ros::Publisher pub;

MyRoboHead::MyRoboHead()
{
    pos_[0] = 0.0; pos_[1] = 0.0;
    vel_[0] = 0.0; vel_[1] = 0.0;
    eff_[0] = 0.0; eff_[1] = 0.0;
    cmd_[0] = 0.0; cmd_[1] = 0.0;

    hardware_interface::JointStateHandle state_handle_1("head_joint", &pos_[0], &vel_[0], &eff_[0]);
    hardware_interface::JointStateHandle state_handle_2("neck_joint",  &pos_[1], &vel_[1], &eff_[1]);
    
    jnt_state_interface_.registerHandle(state_handle_1);
    jnt_state_interface_.registerHandle(state_handle_2);

    registerInterface(&jnt_state_interface_);

    hardware_interface::JointHandle vel_handle_1(jnt_state_interface_.getHandle("head_joint"), &cmd_[0]);
    hardware_interface::JointHandle vel_handle_2(jnt_state_interface_.getHandle("neck_joint"), &cmd_[1]);
    
    jnt_vel_interface_.registerHandle(vel_handle_1);
    jnt_vel_interface_.registerHandle(vel_handle_2);

    registerInterface(&jnt_vel_interface_);
}

void MyRoboHead::read(ros::Time time, ros::Duration period)
{
    if (cmd_[0] || cmd_[1])
    {
    	ROS_INFO_STREAM("Commands for joints: " << cmd_[0] << ", " << -cmd_[1]);
    	
        // 首モータへのシリアル通信
        std_msgs::String msg;
        msg.data = "CML30";    
        pub.publish(msg);
    }
}

void MyRoboHead::write(ros::Time time, ros::Duration period)
{
    // writeは頭の角度を教えなければならないがどうしようか。
    // マイコンから角度を取得することもできるみたいだが、readで指令値を送ってから何秒立ったかで角度を知ることもできる。
    /*
    for (unsigned int i = 0; i < 2; ++i)
    {
        pos_[i] += yp_vel[i] * getPeriod().toSec();
        vel_[i] = yp_vel[i];
    }
    */
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "my_robo_head_control");
    ros::NodeHandle nh;
    pub = nh.advertise<std_msgs::String>("neck_rotate", 10);
    MyRoboHead myrobo;
    controller_manager::ControllerManager cm(&myrobo, nh);

    ros::Rate rate(1.0 / myrobo.getPeriod().toSec());
    ros::AsyncSpinner spinner(1);
    spinner.start();

    while (ros::ok())
    {
        ros::Time now = myrobo.getTime();
        ros::Duration dt = myrobo.getPeriod();
        myrobo.read(now, dt);
        myrobo.write(now, dt);
        cm.update(now, dt);
        rate.sleep();
    }
    
    spinner.stop();
    return 0;
}


