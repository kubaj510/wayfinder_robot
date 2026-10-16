#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <chrono>
#include "wayfinder_interfaces/srv/confirm_marker.hpp"

using namespace std::chrono_literals;
using NavigateToPose = nav2_msgs::action::NavigateToPose;
using ConfirmMarker = wayfinder_interfaces::srv::ConfirmMarker;
using GoalHandleNav = rclcpp_action::ClientGoalHandle<NavigateToPose>;

class Mission : public rclcpp::Node{
    public:
        Mission();

        void send_goal();
    
    private:
        rclcpp_action::Client<NavigateToPose>::SharedPtr client_;
        rclcpp::Client<ConfirmMarker>::SharedPtr aruco_client_;

        std::vector<geometry_msgs::msg::PoseStamped> waypoints_;
        std::vector<std::string> names_;

        void on_marker_confirmed(rclcpp::Client<ConfirmMarker>::SharedFuture future);
        void goal_response_callback(GoalHandleNav::SharedPtr goal_response);
        void feedback_callback(GoalHandleNav::SharedPtr, const std::shared_ptr<const NavigateToPose::Feedback> feedback);
        void result_callback(const GoalHandleNav::WrappedResult &result);
        void aruco_task();

        size_t current_waypoint_index_ = 0;
        rclcpp::TimerBase::SharedPtr retry_timer_;
        int retry_count_ = 0;
        int max_retries_ = 5;
};
