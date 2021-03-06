# SentenceMerge

语料的合并（去重）工具

## Install

	mkdir build && cd build
	conan install .. --build missing
	
	# Linux, Mac 使用 GNU Make
	cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
	cmake --bulid .
	
	# Linux, Mac 使用 Ninja（便宜速度快，优先使用并发编译）
	cmake .. =G "Ninja"
	cmake -- build .
	
	# Windows VS 2017
	cmake .. -G "Visual Studio 15 Win64"
	cmake --build . --config Release
	

## Conan 编译选项

项目编译链接要想成功而不是出现 undefined function，必须修改如下文件：

	vim ~/.conan/profile/default
	libstd++ -> libstd++11
	compiler.libcxx=libstdc++11

Poco 依赖的 OpenSSL 在 ARM 64 位处理器上编译时，需要手动修改以下文件以保证编译通过：
	vim ~/.conan/data/OpenSSL/1.0.2l/conan/stable/export/conanbuild.py

在 142 行后，在此判断后加上两行：

	elif self.settings.arch == "armv8":
	target = "%slinux-aarch64" % target_prefix

## Gcc 版本

本项目依赖的 Poco 项目，<del>使用 Ubuntu Gcc 5.4 编译时会有问题。所以需要确保 Gcc 版本 >= 6。</del>

我使用的 gcc-7。

	sudo add-apt-repository ppa:ubuntu-toolchain-r/test
	sudo apt-get update
	sudo apt-get install gcc-7
	sudo apt-get install g++-7
	sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 20
	sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 20
	sudo update-alternatives --config gcc
	sudo update-alternatives --config g++
