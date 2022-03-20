# BiBiFi Docker Image - handout

# Run using
# docker build --tag bibifi .
# docker run --rm -it bibifi

# This should open a `bash` shell, where the directory containing this
# Dockerfile can be accessed via the `/connect` directory.

FROM ubuntu:18.04
ENV REFRESH_DATE "2021-09-12"

RUN apt-get update
RUN apt-get dist-upgrade -y

# install dependencies for autolab
RUN apt-get update && apt-get install -y build-essential gcc git make sudo

# install dependencies for bibifi
RUN apt-get install -y python3
RUN apt-get install -y software-properties-common
RUN apt-get install -y openssl
RUN apt-get install -y libprotobuf-c0-dev libprotobuf-dev libprotoc-dev protobuf-compiler
RUN apt-get install -y clang
RUN apt-get install -y cmake zlib1g-dev libcppunit-dev llvm
RUN apt-get install -y libssl-dev libssh-dev
RUN apt-get install -y libffi-dev libcrypto++-dev
RUN apt-get install -y libmbedtls-dev
RUN apt-get install -y libnacl-dev
RUN apt-get install -y libsodium-dev
RUN apt-get install -y uthash-dev libjansson-dev libgcrypt11-dev
RUN rm /dev/random && ln -s /dev/urandom /dev/random
RUN rm -rf /var/lib/apt/lists/*

# Install autodriver
WORKDIR /home
RUN useradd autolab
RUN useradd autograde
RUN mkdir autolab autograde output
RUN chown autolab:autolab autolab
RUN chown autolab:autolab output
RUN chown autograde:autograde autograde
RUN git clone --depth 1 https://github.com/autolab/Tango.git
WORKDIR Tango/autodriver
RUN make clean && make
RUN cp autodriver /usr/bin/autodriver
RUN chmod +s /usr/bin/autodriver

# Clean up
WORKDIR /home
RUN apt-get remove -y git && apt-get -y autoremove && rm -rf Tango/

# Check autodriver installation
RUN ls -l /home
RUN which autodriver

# NOT_ON_SERV_START
RUN mkdir /connect
COPY . /connect
WORKDIR /connect
CMD "/bin/bash"
# NOT_ON_SERV_END
