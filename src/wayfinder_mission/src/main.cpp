#include "wayfinder_mission/mission_node.h"
#include <rclcpp/rclcpp.hpp>

int main(int argc, char **argv){
    rclcpp::init(argc, argv);
    
    auto node = std::make_shared<Mission>();
    node->send_goal();

    rclcpp::spin(node);

    return 0;
}
