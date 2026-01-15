cmake_minimum_required(VERSION 3.16)
project(flowfund_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)

# Header-only HTTP server
FetchContent_Declare(httplib
  URL https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(httplib)

# Header-only JSON
FetchContent_Declare(nlohmann_json
  URL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(flowfund
  src/main.cpp
  src/Env.cpp
  src/Db.cpp
  src/Password.cpp
  src/Jwt.cpp
)

target_include_directories(flowfund PRIVATE
  ${httplib_SOURCE_DIR}
  ${nlohmann_json_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(PostgreSQL REQUIRED)
target_include_directories(flowfund PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(flowfund PRIVATE ${PostgreSQL_LIBRARIES})

find_package(OpenSSL REQUIRED)
target_link_libraries(flowfund PRIVATE OpenSSL::Crypto)
