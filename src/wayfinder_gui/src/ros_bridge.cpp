#include "wayfinder_gui/ros_bridge.h"
#include <rclcpp/rclcpp.hpp>
#include <rcl_interfaces/msg/log.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <QObject>
#include <QImage>
#include <QString>

RosBridge::RosBridge(QObject *parent) : QThread(parent){
    node_ = std::make_shared<rclcpp::Node>("ros_bridge");
    
    logSubscription_ = node_->create_subscription<rcl_interfaces::msg::Log>(
            "/rosout", 
            10, 
            std::bind(&RosBridge::logCallback, this, std::placeholders::_1)
    );

    cameraImage_ = node_->create_subscription<sensor_msgs::msg::Image>(
            "camera/image_raw",
            10,
            std::bind(&RosBridge::imageCallback, this, std::placeholders::_1)
    );

}

RosBridge::~RosBridge(){
    if(rclcpp::ok()){
        rclcpp::shutdown();
    }
    wait();
}

void RosBridge::run(){ 
    spin(node_);
}

void RosBridge::logCallback(const rcl_interfaces::msg::Log::SharedPtr msg){
    if(msg->name != "mission_node") return;

    emit logReceived(QString::fromStdString(msg->msg));
}

void RosBridge::imageCallback(const sensor_msgs::msg::Image::SharedPtr msg){
    cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, "rgb8");

    QImage qt_img(cv_ptr->image.data,
                  cv_ptr->image.cols,
                  cv_ptr->image.rows,
                  cv_ptr->image.step,
                  QImage::Format_RGB888);

    emit cameraFrame(qt_img.copy()); 
}

