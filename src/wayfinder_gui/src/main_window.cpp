#include "wayfinder_gui/main_window.h"
#include "wayfinder_gui/ui_mainwindow.h"
#include "wayfinder_gui/ros_bridge.h"

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent), rosBridge_(new RosBridge(this)), ui(new Ui::MainWindow){
    
    ui->setupUi(this);
    setupConnections();

    rosBridge_->start();
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::setupConnections(){
    connect(rosBridge_, &RosBridge::logReceived, this, &MainWindow::appendLogToUi);
    connect(rosBridge_, &RosBridge::cameraFrame, this, &MainWindow::newCameraFrame);
}

void MainWindow::appendLogToUi(const QString &msg){
    QString color = "#cdd6f4";
    QString formatted = QString("<span style='color:%1;'>%2</span>").arg(color, msg.toHtmlEscaped());

    ui->logPlainTextEdit->appendHtml(formatted);
}

void MainWindow::newCameraFrame(const QImage &img){
    QPixmap pixmap = QPixmap::fromImage(img);

    ui->cameraLabel->setPixmap(
        pixmap.scaled(ui->cameraLabel->size(),
                      Qt::KeepAspectRatio,
                      Qt::SmoothTransformation)
    );
}
