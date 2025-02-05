#include <qd_control/qd_hw_interface.h>

Quadrus::Quadrus(ros::NodeHandle& nh): nh_(nh){ //Initialization list : set nh_ to nh

    init();
    controller_manager_.reset(new controller_manager::ControllerManager(this, nh_));

    //Create a timer to periodically call the update() method
    ros::Duration update_freq = ros::Duration(1.0/LOOP_REFRESH_RATE);
    qd_control_loop_ = nh_.createTimer(update_freq, &Quadrus::update, this);

    //Inform master that the node will be publishing to topic /joint_positions with queue size 1000
    cmd_pub = nh_.advertise<std_msgs::Float64MultiArray>("hw_cmd", 10);

    //Subscribe to /feedback_data topic with queue size 1000
    //feedback_sub = nh_.subscribe("hw_feedback", 10, &Quadrus::feedbackCallback, this);
}   
                                                   
Quadrus::~Quadrus(){
    //delete[] jsHandle;
    //delete[] jpHandle;
    //delete[] jlHandle;
}   

void Quadrus::init(){
    
    for(int i = 0;i<NB_JOINTS;i++){
    
        jsHandle[i] = hardware_interface::JointStateHandle(("J" + std::to_string(i+1)), &pos[i], &vel[i], &eff[i]);
        joint_state_interface_.registerHandle(jsHandle[i]);

        jpHandle[i] = hardware_interface::JointHandle(joint_state_interface_.getHandle(("J" + std::to_string(i+1))), &cmd[i]);
        position_joint_interface_.registerHandle(jpHandle[i]);

        /*
        getJointLimits(("J" + std::to_string(i+1)), nh_, jlimits[i]);
        jlHandle[i] = joint_limits_interface::PositionJointSaturationHandle(jpHandle[i], jlimits[i]);
        position_joint_sat_interface.registerHandle(jlHandle[i]);
        */
    }

    registerInterface(&joint_state_interface_);
    registerInterface(&position_joint_interface_);
    //registerInterface(&position_joint_sat_interface);
}

void Quadrus::update(const ros::TimerEvent& e){
    elapsed_time_ = ros::Duration(e.current_real - e.last_real);
    read();
    controller_manager_->update(ros::Time::now(), elapsed_time_);
    write(elapsed_time_);
}  

void Quadrus::read(){
    for(int i=0; i<NB_JOINTS; i++){
        pos[i] = 0;   
    }
}

void Quadrus::write(ros::Duration elapsed_time){
    
    //position_joint_sat_interface.enforceLimits(elapsed_time);
    cmd_array.data.clear();
    for(int i=0;i<NB_JOINTS;i++){
        cmd_array.data.push_back(cmd[i]);
    }
    cmd_pub.publish(cmd_array);

}

void Quadrus::feedbackCallback(const std_msgs::Float64MultiArray& feedback_data){
    
    for(int i=0; i<NB_JOINTS; i++){
        pos[i] = feedback_data.data[i];   
    }
    
}

int main(int argc, char** argv){
    ros::init(argc, argv, "qd_hw_interface");
    ros::NodeHandle nh;
    Quadrus quad(nh);
    
    ros::MultiThreadedSpinner spinner(2);
    spinner.spin();

    return 0;
}