cmake_minimum_required(VERSION 3.10)
project(shortlink-service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(SQLite3 REQUIRED)

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${SQLite3_INCLUDE_DIRS}
    /opt/homebrew/include
)

# 源文件
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 可执行文件
add_executable(shortlink-service ${SOURCES})

# 链接库
target_link_libraries(shortlink-service
    ${SQLite3_LIBRARIES}
    pthread
)

# 单元测试
# option(BUILD_TESTS "Build tests" ON)
# if(BUILD_TESTS)
#     enable_testing()
#     add_subdirectory(tests)
# endif()
