FROM ubuntu:24.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
ARG SISKIN_VERSION=0.23.2
ARG SISKIN_SHA256=58266fb56abbd634282f235b223f9263d9c7f0b0ccc6ee30ea9e45be25764c30

RUN test "$(dpkg --print-architecture)" = amd64 \
    && apt-get update \
    && apt-get install -y --no-install-recommends build-essential ca-certificates curl git \
    && rm -rf /var/lib/apt/lists/*
RUN curl -fsSL --retry 3 \
        "https://github.com/Siskin-framework/Builder/releases/download/${SISKIN_VERSION}/siskin-linux-x86_64.gz" \
        -o /tmp/siskin.gz \
    && echo "${SISKIN_SHA256}  /tmp/siskin.gz" | sha256sum -c - \
    && gunzip -c /tmp/siskin.gz > /usr/local/bin/siskin \
    && chmod +x /usr/local/bin/siskin \
    && rm /tmp/siskin.gz

WORKDIR /src
COPY . .
RUN siskin make/rebol3.nest rebol3-bulk-linux-x64 \
    && siskin make/rebol3.nest test-x64.rebx \
    && cp build/test-x64.rebx src/tests/
RUN build/rebol3-bulk-linux-x64 -v \
    && sh make/docker/check-glibc.sh build/rebol3-bulk-linux-x64

FROM build AS test
ENV CI=true
ENV LD_LIBRARY_PATH=/src/src/tests
RUN sh make/docker/run-tests.sh

# Export only after the tests have completed successfully.
FROM scratch AS artifact
COPY --from=test /src/build/rebol3-bulk-linux-x64 /rebol3-bulk-linux-x64
