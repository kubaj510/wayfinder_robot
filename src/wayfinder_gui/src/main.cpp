#include "wayfinder_gui/main_window.h"
#include <QApplication>
#include <rclcpp/rclcpp.hpp>

int main(int argc, char **argv){
    rclcpp::init(argc, argv);
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    int c = app.exec();

    if(rclcpp::ok()){
        rclcpp::shutdown();
    }

    return c;
}
