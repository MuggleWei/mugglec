ARG OS_VER

FROM alpine:${OS_VER} as builder

RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apk/repositories
RUN apk update
RUN apk add --no-cache build-base
RUN apk add --no-cache git
RUN apk add --no-cache cmake

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
