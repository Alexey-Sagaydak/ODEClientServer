FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    && apt-get clean

WORKDIR /server

CMD ["/bin/bash"]