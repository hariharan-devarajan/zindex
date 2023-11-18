FROM ubuntu:18.04
LABEL authors="hariharandev1"

RUN apt-get update && apt-get install -y gcc-8 g++-8 python3 python3-pip \
    openmpi-bin openmpi-common libopenmpi-dev git cmake zlib1g-dev
ENV CC=gcc-8
ENV CXX=g++-8
RUN pip3 install -U build wheel auditwheel setuptools
RUN mkdir -p /app
RUN echo 1
RUN git clone https://github.com/hariharan-devarajan/zindex.git /app/zindex
ENV ZINDEX_WHEEL=1
RUN cd /app/zindex && python3 setup.py bdist_wheel
