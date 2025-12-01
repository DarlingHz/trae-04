cmake_minimum_required(VERSION 3.16)
project(WatchServer VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 查找依赖
find_package(Threads REQUIRED)

# 第三方库
include_directories(third_party)

# nlohmann/json
include_directories(third_party/nlohmann/include)

# cpp-httplib
include_directories(third_party/cpp-httplib)

# SQLite3
find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

# Google Test
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

# 源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# 可执行文件
add_executable(watch_server ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(watch_server
    Threads::Threads
    ${SQLite3_LIBRARIES}
)

# 测试
if(BUILD_TESTING)
    file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
    add_executable(watch_server_tests ${TEST_SOURCES})
    target_link_libraries(watch_server_tests
        gtest_main
        ${SQLite3_LIBRARIES}
    )
    include(GoogleTest)
    gtest_discover_tests(watch_server_tests)
endif()

# 安装
install(TARGETS watch_server DESTINATION bin)
install(DIRECTORY config/ DESTINATION etc/watch_server)
install(DIRECTORY db/ DESTINATION var/lib/watch_server)
install(DIRECTORY logs/ DESTINATION var/log/watch_server)
