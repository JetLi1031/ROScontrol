#include <roscontrol/roscontrolinter.h>

//namesapce i2c_ros

ROBOTHardwareInterface::ROBOTHardwareInterface(ros::NodeHandle& nh) : nh_(nh) {
    init();
    
    controller_manager_.reset(new controller_manager::ControllerManager(this, nh_));
    loop_hz_=10;
    ros::Duration update_freq = ros::Duration(1.0/loop_hz_);

    batterpub = nh_.advertise<std_msgs::Float64>("/thebatterystatus/percentage", 1000);
	
    non_realtime_loop_ = nh_.createTimer(update_freq, &ROBOTHardwareInterface::update, this);
}

ROBOTHardwareInterface::~ROBOTHardwareInterface() {
}

void ROBOTHardwareInterface::init() {
	motordeclare();

	for(int i=0; i<2; i++)
	{

	// Create joint state interface
        hardware_interface::JointStateHandle jointStateHandle(joint_name_[i], &joint_position_[i], &joint_velocity_[i], &joint_effort_[i]);
        joint_state_interface_.registerHandle(jointStateHandle);
       
    // Create velocity joint interface
	    hardware_interface::JointHandle jointVelocityHandle(jointStateHandle, &joint_velocity_command_[i]);
        velocity_joint_interface_.registerHandle(jointVelocityHandle);

    // Create Joint Limit interface   
        joint_limits_interface::JointLimits limits;
        joint_limits_interface::getJointLimits(joint_name_[i], nh_, limits);
	    joint_limits_interface::VelocityJointSaturationHandle jointLimitsHandle(jointVelocityHandle, limits);
	    velocityJointSaturationInterface.registerHandle(jointLimitsHandle);

	}
    
// Register all joints interfaces    
    registerInterface(&joint_state_interface_);
    registerInterface(&velocity_joint_interface_);
    registerInterface(&velocityJointSaturationInterface);
}

void ROBOTHardwareInterface::update(const ros::TimerEvent& e) {
    elapsed_time_ = ros::Duration(e.current_real - e.last_real);
    read();
    controller_manager_->update(ros::Time::now(), elapsed_time_);
    write(elapsed_time_);
}

void ROBOTHardwareInterface::read() {
    double leftside,rightside,battery;
    
    buffread(leftside , rightside,battery);
    left_motor_pos+=angles::from_degrees(leftside);
    joint_position_[0]=left_motor_pos;
    
    right_motor_pos+=angles::from_degrees(rightside);
    joint_position_[1]=right_motor_pos;
    
    std_msgs::Float64 msg; 
    msg.data = battery;
    batterpub.publish(msg);
    
    //ROS_INFO("pos=%.2f x=%d ",pos,x);
	
}

void ROBOTHardwareInterface::write(ros::Duration elapsed_time) {
   
    velocityJointSaturationInterface.enforceLimits(elapsed_time);   

	uint8_t wbuff[2];

    int velocity,result;
    
    
    velocity=(int)angles::to_degrees(joint_velocity_command_[0]);
	//ROS_INFO("joint_velocity_command_[0]=%.2f velocity=%d  B1=%d B2=%d", joint_velocity_command_[0],velocity,wbuff[0],wbuff[1]);

    if(left_prev_cmd!=velocity)
    {
        buffwrite('L', velocity);
	    //ROS_INFO("Writen successfully Left result=%d", velocity);
	    left_prev_cmd=velocity;
    }
    
    velocity=(int)angles::to_degrees(joint_velocity_command_[1]);
	//ROS_INFO("joint_velocity_command_[0]=%.2f velocity=%d  B1=%d B2=%d", joint_velocity_command_[0],velocity,wbuff[0],wbuff[1]);

    if(right_prev_cmd!=velocity)
    {
	    buffwrite('R', velocity);
	    //ROS_INFO("Writen successfully right result=%d", velocity);
	    right_prev_cmd=velocity;
    }


		
}



int main(int argc, char** argv)
{
    ros::init(argc, argv, "controllerinterface");
    ros::NodeHandle nh;
    //ros::AsyncSpinner spinner(4); 
     
    ros::MultiThreadedSpinner spinner(2); // Multiple threads for controller service callback and for the Service client callback used to get the feedback from ardiuno
    ROBOTHardwareInterface ROBOT(nh);
    //spinner.start();
    spinner.spin();
    //ros::spin();
    return 0;
}
