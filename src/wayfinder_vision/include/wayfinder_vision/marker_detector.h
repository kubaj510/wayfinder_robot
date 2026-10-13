#pragma once
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/int32_multi_array.hpp"
#include "wayfinder_interfaces/srv/confirm_marker.hpp"
#include <opencv2/aruco.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <mutex>

using ConfirmMarker = wayfinder_interfaces::srv::ConfirmMarker;

class MarkerDetector : public rclcpp::Node{
    public:
        MarkerDetector();        

    private:
        rclcpp::Service<ConfirmMarker>::SharedPtr service_;
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;

        cv::Mat latest_frame_;
        std::mutex mutex_;
        bool has_frame_ = false;

        cv::Ptr<cv::aruco::Dictionary> dict_;
        cv::Ptr<cv::aruco::DetectorParameters> detecor_params_;


        void detect_marker(const std::shared_ptr<ConfirmMarker::Request> req,
                           std::shared_ptr<ConfirmMarker::Response> res);

        void image_callback(const sensor_msgs::msg::Image::SharedPtr msg);
};
