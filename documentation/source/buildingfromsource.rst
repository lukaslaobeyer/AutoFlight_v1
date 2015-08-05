Building AutoFlight from Source
*******************************

On Ubuntu 14.04 LTS
===================

Dependencies
++++++++++++

General needed packages
-----------------------

::

    sudo apt-get update
    sudo apt-get upgrade -y
    sudo apt-get install -y build-essential autoconf automake pkg-config
    sudo apt-get install -y gcc-arm-linux-gnueabi
    sudo apt-get install -y cmake git curl libyaml-cpp-dev
    sudo apt-get install -y python3 python3-pip python3-dev python3-numpy
    sudo apt-get install -y libboost1.55-all-dev
    sudo pip3 install Sphinx

FFmpeg
------

Dependencies:

::

    sudo apt-get install -y yasm libtool libx264-dev

Build:

::

    mkdir ~/libs
    cd ~/libs
    git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg
    cd ffmpeg
    ./configure \
      --enable-shared \
      --prefix="$HOME/libs/ffmpeg/build" \
      --extra-cflags="-I$HOME/libs/ffmpeg/build/include" \
      --extra-ldflags="-L$HOME/libs/ffmpeg/build/lib" \
      --bindir="$HOME/libs/ffmpeg/build/bin" \
      --enable-gpl \
      --enable-libx264
    make
    make install
    make distclean
    sudo mkdir /opt/ffmpeg
    sudo cp -R build/* /opt/ffmpeg

Qt 5
----

::

    sudo apt-get install -y qtbase5-dev libqt5webkit5-dev libqt5opengl5-dev qt5-default

OpenCV
------

Dependencies:

::

    sudo apt-get install -y libjpeg-dev libpng-dev libtiff-dev

Build:

::

    cd ~/libs
    mkdir opencv
    cd opencv
    wget https://github.com/Itseez/opencv/archive/3.0.0.tar.gz
    tar -zxvf 3.0.0.tar.gz
    cd opencv-3.0.0
    mkdir build
    cd build
    cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/opt/opencv -D BUILD_opencv_python2=OFF -DBUILD_opencv_python3=ON -D WITH_QT=ON ..
    make
    sudo make install

Eigen 3
-------

::

    sudo apt-get install -y libeigen3-dev

Premake 5
---------

::

    mkdir ~/.premake
    cd ~/.premake
    wget https://github.com/premake/premake-core/releases/download/v5.0.0.alpha4/premake-5.0.0.alpha4-linux.tar.gz
    tar -zxvf premake-5.0.0.alpha4-linux.tar.gz


Add premake to PATH:

::

    echo "export PATH=$HOME/.premake:$PATH" >> ~/.bashrc
    source ~/.bashrc
