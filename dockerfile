FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    make \
    git \
    wget \
    ca-certificates \
    redis-tools \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app

COPY . /app

EXPOSE 6379

CMD ["/bin/bash"]