#include "wayfinder_vision/marker_detector.h"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/int32_multi_array.hpp"
#include "wayfinder_interfaces/srv/confirm_marker.hpp"
#include <opencv2/aruco.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <mutex>

MarkerDetector::MarkerDetector() : Node("MarkerDetector"){
    service_ = this->create_service<ConfirmMarker>(
        "detect_marker", 
        std::bind(&MarkerDetector::detect_marker, this, std::placeholders::_1, std::placeholders::_2)
    );
    image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
        "/camera/image_raw", 
        rclcpp::SensorDataQoS(),
        std::bind(&MarkerDetector::image_callback, this, std::placeholders::_1)
    );

    detecor_params_ = cv::aruco::DetectorParameters::create();
    dict_ = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50); 
}


void MarkerDetector::detect_marker(const std::shared_ptr<ConfirmMarker::Request> req,
                                   std::shared_ptr<ConfirmMarker::Response> res){
            
    cv::Mat frame_to_process;
    {
        std::lock_guard<std::mutex> lock(mutex_);
                
        if(!has_frame_ || latest_frame_.empty()){
            RCLCPP_WARN(this->get_logger(), "service called but no camera frame has been received yet");
            res->success= false;
            return;
        }

        frame_to_process = latest_frame_.clone();
    }

    std::vector<std::vector<cv::Point2f>> marker_corners;
    std::vector<int> marker_ids;

    cv::aruco::detectMarkers(frame_to_process, dict_, marker_corners, marker_ids, detecor_params_);
            
    auto it = std::find(marker_ids.begin(), marker_ids.end(), req->expected_marker_id);
    if(it != marker_ids.end()){
        res->success = true;

    } else{
        res->success = false;
    }

}

void MarkerDetector::image_callback(const sensor_msgs::msg::Image::SharedPtr msg){
    cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, "bgr8");

    {
        std::lock_guard<std::mutex> lock(mutex_);
        latest_frame_ = cv_ptr->image;
        has_frame_ = true;
    }
}
