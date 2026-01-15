cmake_minimum_required(VERSION 3.16)
project(flowfund LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(PostgreSQL REQUIRED)

add_executable(flowfund
  src/main.cpp
  src/Env.cpp
  src/Db.cpp
  src/Password.cpp
  src/Jwt.cpp
)

target_include_directories(flowfund PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(flowfund PRIVATE
  PostgreSQL::PostgreSQL
  OpenSSL::SSL
  OpenSSL::Crypto
)
