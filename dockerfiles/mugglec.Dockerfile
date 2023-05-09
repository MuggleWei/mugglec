ARG OS
ARG OS_VER
ARG REGISTRY

# step 1
FROM ${REGISTRY}/${OS}-dev:${OS_VER} as builder

RUN mkdir -p /app/src/mugglec
RUN mkdir -p /app/src/mugglec/build
COPY ./CMakeLists.txt /app/src/mugglec/
COPY ./version.txt /app/src/mugglec/
COPY ./muggle /app/src/mugglec/muggle
COPY ./test_utils /app/src/mugglec/test_utils
COPY ./test /app/src/mugglec/test
COPY ./cmake /app/src/mugglec/cmake
COPY ./LICENSE /app/src/mugglec/LICENSE
WORKDIR /app/src/mugglec
RUN cmake \
	-S /app/src/mugglec \
	-B /app/src/mugglec/build \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_PREFIX_PATH=/opt/googletest \
	-DCMAKE_INSTALL_PREFIX=/usr/local \
	-DBUILD_SHARED_LIBS=ON \
	-DMUGGLE_BUILD_STATIC_PIC=ON \
	-DMUGGLE_BUILD_TRACE=OFF \
	-DMUGGLE_BUILD_TESTING=ON \
	-DMUGGLE_BUILD_EXAMPLE=OFF \
	-DMUGGLE_BUILD_BENCHMARK=OFF \
	-DMUGGLE_INSTALL_BIN=OFF
RUN cmake --build /app/src/mugglec/build
RUN cmake --build /app/src/mugglec/build --target test
RUN cmake --build /app/src/mugglec/build --target install

# step 2
FROM ${OS}:${OS_VER}
RUN mkdir -p /usr/local
COPY --from=builder /usr/local /usr/local
