ARG UBUNTU_VERSION=18.04
# ARG UBUNTU_VERSION=24.04

FROM ubuntu:${UBUNTU_VERSION}

RUN apt-get update && \
    apt-get install -y \
        git \
        gcc \
        g++ \
        make \
        gdb \
        vim \
        wget \
        python \
        python-dev \
        python-setuptools \
        # python3 \
        # python3-dev \
        # python3-setuptools \
        autoconf \
        flex \
        bison \
        libfl-dev \
        tcpdump \
        gsl-bin \
        libgslcblas0 \
        libgsl-dev \
        sqlite3 \
        libsqlite3-dev \
        libxml2-dev \
        cmake \
        libc6-dev \
        libboost-all-dev \
        # libboost-signals-dev \
        # libboost-filesystem-dev \
        gnuplot \
        bzip2 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ARG NS3_VERSION=3.29
# ARG NS3_VERSION=3.43

RUN mkdir -p /usr/ns3
WORKDIR /usr 

RUN wget https://www.nsnam.org/release/ns-allinone-${NS3_VERSION}.tar.bz2  && \
    tar -jxvf ns-allinone-${NS3_VERSION}.tar.bz2

RUN cd ns-allinone-${NS3_VERSION} && ./build.py --enable-examples --enable-tests

RUN ln -s /usr/ns-allinone-${NS3_VERSION}/ns-${NS3_VERSION}/ /usr/ns3/

RUN rm ns-allinone-${NS3_VERSION}.tar.bz2

WORKDIR /usr/ns-allinone-${NS3_VERSION}/ns-${NS3_VERSION}

# If any other files is required inside the docker container, add more lines as below, changing the filenames and paths as required.
# COPY {filename/path to file on host} {path to where file should be copied to in the constainer}
COPY IKT443_NS3_codes.cc /usr/ns-allinone-${NS3_VERSION}/ns-${NS3_VERSION}/scratch/
COPY myapp.h /usr/ns-allinone-${NS3_VERSION}/ns-${NS3_VERSION}/scratch/

CMD tail -f /dev/null