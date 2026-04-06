FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ARG OIDN_VERSION=2.3.0
ARG ISPC_VERSION=1.24.0

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    git-lfs \
    wget \
    libtbb-dev \
    libopenimageio-dev \
    openimageio-tools \
    libpng-dev \
    libjpeg-dev \
    libtiff-dev \
    && rm -rf /var/lib/apt/lists/*

# Download and install ISPC
RUN wget -q https://github.com/ispc/ispc/releases/download/v${ISPC_VERSION}/ispc-v${ISPC_VERSION}-linux.tar.gz \
    && tar -xzf ispc-v${ISPC_VERSION}-linux.tar.gz \
    && mv ispc-v${ISPC_VERSION}-linux/bin/ispc /usr/local/bin/ispc \
    && rm -rf ispc-v${ISPC_VERSION}-linux*

# Clone OIDN source with LFS for model weights
RUN git lfs install && \
    git clone --recursive --depth 1 --branch v${OIDN_VERSION} \
    https://github.com/RenderKit/oidn.git /oidn

# Build OIDN with OpenImageIO enabled
RUN cmake -S /oidn -B /oidn/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/opt/oidn \
    -DOIDN_APPS=ON \
    -DOIDN_APPS_OPENIMAGEIO=ON \
    -DOIDN_DEVICE_CPU=ON \
    && cmake --build /oidn/build --parallel $(nproc) \
    && cmake --install /oidn/build

# Stage 2: Lean runtime image
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libgomp1 \
    libtbb12 \
    libopenimageio2.4t64 \
    libpng16-16t64 \
    libjpeg8 \
    libtiff6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /opt/oidn /opt/oidn

ENV PATH="/opt/oidn/bin:$PATH"
ENV LD_LIBRARY_PATH="/opt/oidn/lib:$LD_LIBRARY_PATH"

WORKDIR /images
ENTRYPOINT ["oidnDenoise"]
CMD ["--help"]
