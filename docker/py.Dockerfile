FROM ubuntu:20.04
LABEL authors="hariharandev1"
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y gcc g++ python3 python3-pip \
    openmpi-bin openmpi-common libopenmpi-dev git cmake zlib1g-dev
RUN pip3 install -U build wheel auditwheel setuptools
RUN mkdir -p /app
RUN git clone https://github.com/hariharan-devarajan/zindex.git /app/zindex
ENV ZINDEX_WHEEL=1
RUN cd /app/zindex && python3 setup.py bdist_wheel
