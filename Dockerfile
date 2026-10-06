FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git python3 nodejs stockfish ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake --preset linux && cmake --build --preset linux -j
RUN ctest --preset linux