ARG OS_VER

FROM ubuntu:${OS_VER} as builder
RUN sed -i "s@http://.*archive.ubuntu.com@http://mirrors.tuna.tsinghua.edu.cn@g" /etc/apt/sources.list
RUN sed -i "s@http://.*security.ubuntu.com@http://mirrors.tuna.tsinghua.edu.cn@g" /etc/apt/sources.list
RUN apt-get update
RUN apt-get install -y ca-certificates
RUN sed -i "s@http@https@g" /etc/apt/sources.list
RUN apt-get update
RUN apt-get install -y build-essential
RUN apt-get install -y git
RUN apt-get install -y cmake

# install googletest
RUN mkdir -p /usr/local
RUN mkdir -p /app/src/
RUN git clone \
	--depth=1\
	--branch=release-1.12.1 \
	https://github.com/google/googletest.git \
	/app/src/googletest
RUN mkdir -p /app/src/googletest/build
RUN cmake \
	-S /app/src/googletest \
	-B /app/src/googletest/build \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX=/opt/googletest \
	-DBUILD_SHARED_LIBS=ON
RUN cmake --build /app/src/googletest/build --target install
