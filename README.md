# SentenceMerge

语料的合并（去重）工具

## Install

mkdir build && cd build
conan install .. --build missing
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --bulid .

## Gcc 版本

本项目依赖的 Poco 项目，使用 Ubuntu Gcc 5.4 编译时会有问题。所以需要确保 Gcc 版本 >= 6

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-6
sudo apt-get install g++-6
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 20
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 20
sudo update-alternatives --config gcc
sudo update-alternatives --config g++
