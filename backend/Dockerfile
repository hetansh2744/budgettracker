FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
  build-essential cmake curl git \
  libpq-dev \
  libssl-dev \
  ca-certificates \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

# Download header-only dependencies
RUN mkdir -p /app/third_party/nlohmann \
  && curl -L -o /app/third_party/httplib.h \
     https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h \
  && curl -L -o /app/third_party/nlohmann/json.hpp \
     https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp

RUN mkdir -p build && cd build && cmake .. && cmake --build . -j

EXPOSE 10000
CMD ["/app/build/flowfund"]
