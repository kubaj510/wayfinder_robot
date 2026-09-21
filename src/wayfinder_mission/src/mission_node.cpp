#include "wayfinder_mission/mission_node.h"

Mission::Mission() : Node("mission_node"){
    client_ = rclcpp_action::create_client<FollowWaypoints>(this, "follow_waypoints");

    auto names = this->declare_parameter<std::vector<std::string>>("checkpoint_names");
    auto x = this->declare_parameter<std::vector<double>>("checkpoint_x");
    auto y = this->declare_parameter<std::vector<double>>("checkpoint_y");
    names_ = names;

    if(names.size() != x.size() || names.size() != y.size()){
        RCLCPP_ERROR(this->get_logger(), "parameter arrays for checkpoints are unequal size!");
        
        throw std::runtime_error("mismatched checkpoint parameter sizes");
    }

    for(size_t i = 0; i < names_.size(); i++){
        geometry_msgs::msg::PoseStamped pose;
        pose.header.frame_id = "map";
        pose.pose.position.x = x[i];
        pose.pose.position.y = y[i];
        pose.pose.orientation.w = 1.0;

        waypoints_.push_back(pose);
    }
}

void Mission::send_goal(){
    while(!client_->wait_for_action_server(5s)){
        if(!rclcpp::ok()){
            RCLCPP_INFO(this->get_logger(), "mission has been canceled");
            return;
        }
        RCLCPP_INFO(this->get_logger(), "waiting for action server");
    }

    auto goal_msg = FollowWaypoints::Goal();
    goal_msg.poses = waypoints_;

    auto options = rclcpp_action::Client<FollowWaypoints>::SendGoalOptions();
    options.goal_response_callback = std::bind(&Mission::goal_response_callback, this, std::placeholders::_1);
    options.feedback_callback = std::bind(&Mission::feedback_callback, this, std::placeholders::_1, std::placeholders::_2);
    options.result_callback = std::bind(&Mission::result_callback, this, std::placeholders::_1);

    client_->async_send_goal(goal_msg, options);
}

void Mission::goal_response_callback(GoalHandleNav::SharedPtr goal_response){
    if(!goal_response){
        RCLCPP_WARN(this->get_logger(), "goal has been rejected");

    } else{
        RCLCPP_INFO(this->get_logger(), "goal has been accepted");
    }
}

void Mission::feedback_callback(GoalHandleNav::SharedPtr, const std::shared_ptr<const FollowWaypoints::Feedback> feedback){
    RCLCPP_INFO(this->get_logger(), "heading to checkpoint %u: %s",
                feedback->current_waypoint, names_[feedback->current_waypoint].c_str());
}

void Mission::result_callback(const GoalHandleNav::WrappedResult &result){
    switch(result.code){
        case rclcpp_action::ResultCode::SUCCEEDED:
            RCLCPP_INFO(this->get_logger(), "mission complete, %zu checkpoint(s) missed",
               result.result->missed_waypoints.size());
            break;
        
        case rclcpp_action::ResultCode::CANCELED:
            RCLCPP_WARN(this->get_logger(), "action was canceled");
            break;

        case rclcpp_action::ResultCode::ABORTED:
            RCLCPP_ERROR(this->get_logger(), "action was aborted");
            break;

        default:
            RCLCPP_ERROR(this->get_logger(), "unknown action result");
            break;
    }

    rclcpp::shutdown();
}



