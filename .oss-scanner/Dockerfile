# OSS Scanner build image for Oldes/Rebol3
# Place at: .oss-scanner/Dockerfile (and set `dockerfile: .oss-scanner/Dockerfile` in project.yaml)

FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG SISKIN_VERSION=0.22.5
ARG REBOL_REPO=https://github.com/Oldes/Rebol3.git
ARG REBOL_BRANCH=matrix

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        curl \
        git \
    && rm -rf /var/lib/apt/lists/*

# Siskin Builder
RUN curl -fsSL "https://github.com/Siskin-framework/Builder/releases/download/${SISKIN_VERSION}/siskin-linux-x86_64.gz" \
        | gunzip > /usr/local/bin/siskin \
    && chmod +x /usr/local/bin/siskin

# Sources: build context is the repository root
COPY . /src
WORKDIR /src

# Build
RUN siskin make/rebol3.nest -o r3 rebol3-bulk-linux-x64

# Put the binary on PATH, wherever Siskin wrote it
RUN R3="$(find . -type f -name r3 -perm -u+x | head -n 1)" \
    && test -n "$R3" \
    && ln -sf "$(realpath "$R3")" /usr/local/bin/r3

# Sanity check
RUN r3 --version

# Full tests
# RUN r3 -s ./src/tests/run-tests.r3

