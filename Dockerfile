FROM ubuntu:18.10

# Prepare system
RUN apt update

# Common tools
RUN apt install -y git

# CPP - build environment
RUN apt install -y gcc g++ clang cmake