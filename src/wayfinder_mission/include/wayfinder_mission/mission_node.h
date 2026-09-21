#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <nav2_msgs/action/follow_waypoints.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <chrono>

using namespace std::chrono_literals;
using FollowWaypoints = nav2_msgs::action::FollowWaypoints;
using GoalHandleNav = rclcpp_action::ClientGoalHandle<FollowWaypoints>;

class Mission : public rclcpp::Node{
    public:
        Mission();

        void send_goal();
    
    private:
        rclcpp_action::Client<FollowWaypoints>::SharedPtr client_;

        std::vector<geometry_msgs::msg::PoseStamped> waypoints_;
        std::vector<std::string> names_;

        void goal_response_callback(GoalHandleNav::SharedPtr goal_response);
        void feedback_callback(GoalHandleNav::SharedPtr, const std::shared_ptr<const FollowWaypoints::Feedback> feedback);
        void result_callback(const GoalHandleNav::WrappedResult &result);
};
