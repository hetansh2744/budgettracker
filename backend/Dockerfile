FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
  build-essential cmake curl git \
  libpq-dev \
  libssl-dev \
  ca-certificates \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

RUN mkdir -p build && cd build && cmake .. && cmake --build . -j

EXPOSE 10000
CMD ["./build/flowfund"]
