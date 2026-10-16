#pragma once
#include <QMainWindow>
#include "wayfinder_gui/ros_bridge.h"

namespace Ui{
    class MainWindow;
}

class MainWindow : public QMainWindow{
    Q_OBJECT
    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        void setupConnections();
        void appendLogToUi(const QString &msg);
        void newCameraFrame(const QImage &img);

        RosBridge *rosBridge_;
        Ui::MainWindow *ui;
};
