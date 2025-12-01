cmake_minimum_required(VERSION 3.15)
project(AccountingServer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的依赖项
find_package(Boost 1.89 REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# 包含头文件目录
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
    src/
)

# 收集源文件
file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/controllers/*.cpp
    src/services/*.cpp
    src/dao/*.cpp
    src/models/*.cpp
    src/utils/*.cpp
)

# 创建可执行文件
add_executable(accounting_server ${SOURCES})

# 链接依赖项
target_link_libraries(accounting_server
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
    nlohmann_json::nlohmann_json
    pthread
)

# 单元测试
find_package(Catch2 REQUIRED)
file(GLOB TEST_SOURCES tests/*.cpp)
add_executable(accounting_tests ${TEST_SOURCES})

# 包含Catch2头文件目录
target_include_directories(accounting_tests PRIVATE ${Catch2_INCLUDE_DIRS})

# 链接依赖项
target_link_libraries(accounting_tests
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
    nlohmann_json::nlohmann_json
    Catch2::Catch2
    pthread
)

# 启用测试
enable_testing()
add_test(NAME AccountingTests COMMAND accounting_tests)

# 安装目标
install(TARGETS accounting_server DESTINATION /usr/local/bin)
