#include "wayfinder_mission/mission_node.h"

Mission::Mission() : Node("mission_node"){
    client_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");
    aruco_client_ = this->create_client<ConfirmMarker>("detect_marker");

    auto names = this->declare_parameter<std::vector<std::string>>("checkpoint_names");
    auto x = this->declare_parameter<std::vector<double>>("checkpoint_x");
    auto y = this->declare_parameter<std::vector<double>>("checkpoint_y");
    auto yaw = this->declare_parameter<std::vector<double>>("checkpoint_yaw");
    names_ = names;

    if(names.size() != x.size() || names.size() != y.size() || names.size() != yaw.size()){
        RCLCPP_ERROR(this->get_logger(), "parameter arrays for checkpoints are unequal size!");
        
        throw std::runtime_error("mismatched checkpoint parameter sizes");
    }

    for(size_t i = 0; i < names_.size(); i++){
        geometry_msgs::msg::PoseStamped pose;
        pose.header.frame_id = "map";
        pose.pose.position.x = x[i];
        pose.pose.position.y = y[i];
        pose.pose.orientation.z = std::sin(yaw[i] / 2.0);
        pose.pose.orientation.w = std::cos(yaw[i] / 2.0);

        waypoints_.push_back(pose);
    }
}

void Mission::send_goal(){

    if(current_waypoint_index_ >= waypoints_.size()){
        RCLCPP_INFO(this->get_logger(), "all checkpoints reached");
        rclcpp::shutdown();
        return;
    }

    while(!client_->wait_for_action_server(5s)){
        if(!rclcpp::ok()){
            RCLCPP_INFO(this->get_logger(), "mission canceled while waiting for action server");
            return;
        }
        RCLCPP_INFO(this->get_logger(), "waiting for action server");
    }

    auto goal_msg = NavigateToPose::Goal();
    goal_msg.pose.header.stamp = this->now();
    goal_msg.pose = waypoints_[current_waypoint_index_]; 

    auto options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
    options.goal_response_callback = std::bind(&Mission::goal_response_callback, this, std::placeholders::_1);
    options.feedback_callback = std::bind(&Mission::feedback_callback, this, std::placeholders::_1, std::placeholders::_2);
    options.result_callback = std::bind(&Mission::result_callback, this, std::placeholders::_1);

    client_->async_send_goal(goal_msg, options);
}

void Mission::goal_response_callback(GoalHandleNav::SharedPtr goal_response){
    if(!goal_response){
        if(!rclcpp::ok()) return;

        if(retry_count_ < max_retries_){
            retry_count_++;
            RCLCPP_INFO(this->get_logger(), "goal rejected, retry %d/%d", retry_count_, max_retries_);
            retry_timer_ = this->create_wall_timer(
                std::chrono::seconds(2), 
                [this](){
                    retry_timer_->cancel();
                    retry_timer_.reset();
                    send_goal();
                }
            );

        } else{
            RCLCPP_WARN(this->get_logger(), "goal has been rejected");
            rclcpp::shutdown();
        }
    } else{
        RCLCPP_INFO(this->get_logger(), "goal has been accepted");
        retry_count_ = 0; 
    }
}

void Mission::feedback_callback(GoalHandleNav::SharedPtr, const std::shared_ptr<const NavigateToPose::Feedback> feedback){
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 3000, "remaining distance: %f,", feedback->distance_remaining);
}

void Mission::result_callback(const GoalHandleNav::WrappedResult &result){
    switch(result.code){
        case rclcpp_action::ResultCode::SUCCEEDED:
            RCLCPP_INFO(this->get_logger(), "navigation succeeded");
            aruco_task();
            break;
        
        case rclcpp_action::ResultCode::CANCELED:
            RCLCPP_WARN(this->get_logger(), "navigation canceled");
            break;

        case rclcpp_action::ResultCode::ABORTED:
            RCLCPP_ERROR(this->get_logger(), "navigation aborted");
            break;

        default:
            RCLCPP_ERROR(this->get_logger(), "unknown action result");
            break;
    }
}

void Mission::aruco_task(){
    if(!aruco_client_->service_is_ready()){
        RCLCPP_ERROR(this->get_logger(), "marker detection service is not available, try again later");
        return;
    }

    auto req = std::make_shared<ConfirmMarker::Request>();
    req->expected_marker_id = current_waypoint_index_ + 1; 

    aruco_client_->async_send_request(req, std::bind(&Mission::on_marker_confirmed, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "request to verify ArUco marker");
    //current_waypoint_index_++;
    //send_goal();
}

void Mission::on_marker_confirmed(rclcpp::Client<ConfirmMarker>::SharedFuture future){
    auto response = future.get();
    if(response->success){
        RCLCPP_INFO(this->get_logger(), "checkpoint '%s' confirmed", names_[current_waypoint_index_].c_str());

    } else{
        RCLCPP_WARN(this->get_logger(), "checkpoint '%s' missed", names_[current_waypoint_index_].c_str());
    }

    current_waypoint_index_++;
    send_goal();
}
