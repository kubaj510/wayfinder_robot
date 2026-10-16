FROM osrf/ros:jazzy-desktop-full

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    ros-jazzy-ros-gz \
    ros-jazzy-xacro \
    ros-jazzy-gz-ros2-control \
    ros-jazzy-ros2-controllers \
    ros-jazzy-nav2-bringup \
    ros-jazzy-slam-toolbox \
    ros-jazzy-vision-opencv \
    ros-jazzy-teleop-twist-keyboard \
    ros-jazzy-twist-stamper \
    qt6-base-dev \
    libqt6widgets6 \
    python3-colcon-common-extensions \
    git \
    sudo \
    mesa-utils \
    && rm -rf /var/lib/apt/lists/* 

ARG USER_UID=1000
ARG USER_GID=1000

RUN userdel -r ubuntu || true
RUN groupdel ubuntu || true

RUN if getent group ${USER_GID}; then \
        groupmod -n rosuser $(getent group ${USER_GID} | cut -d: -f1); \
    else \
        groupadd -g ${USER_GID} rosuser; \
    fi


RUN useradd -u ${USER_UID} -g ${USER_GID} -s /bin/bash -m rosuser && \
    echo "rosuser ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER rosuser
WORKDIR /workspace

RUN echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
