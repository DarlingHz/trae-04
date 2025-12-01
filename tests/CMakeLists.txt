cmake_minimum_required(VERSION 3.10)

# 查找 Google Test 框架
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 收集所有测试源文件
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# 创建测试可执行文件
add_executable(watch_server_tests ${TEST_SOURCES})

# 链接依赖库
target_link_libraries(watch_server_tests ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(watch_server_tests SQLite3::SQLite3)
target_link_libraries(watch_server_tests nlohmann_json::nlohmann_json)

# 添加测试到 CTest
add_test(NAME watch_server_tests COMMAND watch_server_tests)

# 设置输出目录
set_target_properties(watch_server_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
