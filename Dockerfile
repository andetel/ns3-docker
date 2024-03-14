ARG UBUNTU_VERSION=18.04

FROM ubuntu:${UBUNTU_VERSION}

RUN apt-get update && \
    apt-get install -y \
        git \
        mercurial \
        gcc \
        g++ \
        synaptic \
        vim \
        wget \
        python \
        python-dev \
        python-setuptools \
        qt5-default \
        python-pygraphviz \
        python-kiwi \
        ipython \
        autoconf \
        cvs \
        bzr \
        unrar \
        gdb \
        valgrind \
        uncrustify \
        flex \
        bison \
        libfl-dev \
        tcpdump \
        gsl-bin \
        libgslcblas0 \
        libgsl-dev \
        sqlite \
        sqlite3 \
        libsqlite3-dev \
        libxml2 \
        libxml2-dev \
        cmake \
        libc6-dev \
        libc6-dev-i386 \
        libclang-dev \
        llvm-dev \
        automake \
        libgtk2.0-0 \
        libgtk2.0-dev \
        vtun \
        lxc \
        libboost-signals-dev \
        libboost-filesystem-dev \
        lsb-release \
        mesa-utils \
        gnuplot \
        libgl1-mesa-glx \
        libgl1-mesa-dri \
        mercurial

ARG NS3_VERSION=3.29

ENV DISPLAY=host.docker.internal:0.0
ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES all

# ENV LIBGL_ALWAYS_SOFTWARE=1
ENV LIBGL_ALWAYS_INDIRECT=1

ENV XDG_RUNTIME_DIR=/tmp/runtime-root

RUN mkdir -p ${XDG_RUNTIME_DIR} && chmod 0700 ${XDG_RUNTIME_DIR}

RUN mkdir -p /usr/ns3
WORKDIR /usr 

RUN wget https://www.nsnam.org/release/ns-allinone-${NS3_VERSION}.tar.bz2  && \
    tar -jxvf ns-allinone-${NS3_VERSION}.tar.bz2

RUN cd ns-allinone-${NS3_VERSION} && ./build.py --enable-examples --enable-tests

RUN ln -s /usr/ns-allinone-${NS3_VERSION}/ns-${NS3_VERSION}/ /usr/ns3/

RUN apt-get clean && \
    rm -rf /var/lib/apt && \
    rm ns-allinone-${NS3_VERSION}.tar.bz2

RUN cd /usr/ns-allinone-${NS3_VERSION}/netanim-3.108/ && \
    make clean && \
    qmake NetAnim.pro && \
    make

WORKDIR /usr/ns-allinone-${NS3_VERSION}/ns-${NS3_VERSION}

# If any other files is required inside the docker container, add more lines as below, changing the filenames and paths as required.
# COPY {filename/path to file on host} {path to where file should be copied to in the constainer}
COPY IKT443_NS3_codes.cc /usr/ns-allinone-${NS3_VERSION}/ns-${NS3_VERSION}/scratch/
COPY myapp.h /usr/ns-allinone-${NS3_VERSION}/ns-${NS3_VERSION}/scratch/

CMD tail -f /dev/null