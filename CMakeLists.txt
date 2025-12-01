cmake_minimum_required(VERSION 3.10)
project(api_quota_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找必要的库
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include ${nlohmann_json_INCLUDE_DIRS})

# 收集所有源文件
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# 生成可执行文件
add_executable(api-quota-server ${SOURCES})

# 链接库
target_link_libraries(api-quota-server Threads::Threads nlohmann_json::nlohmann_json sqlite3)

# 安装目标
install(TARGETS api-quota-server DESTINATION bin)
