FROM ubuntu:devel

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-16 g++-16 cmake make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
