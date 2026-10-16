#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rcl_interfaces/msg/log.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <QThread>
#include <QObject>

class RosBridge : public QThread{
    Q_OBJECT
    public:
        RosBridge(QObject *parent = nullptr);
        ~RosBridge();

    protected:
        void run() override;

    private:
        rclcpp::Node::SharedPtr node_;
        rclcpp::Subscription<rcl_interfaces::msg::Log>::SharedPtr logSubscription_;
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr cameraImage_;

        void logCallback(const rcl_interfaces::msg::Log::SharedPtr msg);
        void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg);

    signals:
        void logReceived(const QString &msg);
        void cameraFrame(const QImage &img);
}; 
