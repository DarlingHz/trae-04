cmake_minimum_required(VERSION 3.15)
project(team_task_manager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置调试信息
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# 设置CMP0167策略，以解决FindBoost模块已移除的警告
cmake_policy(SET CMP0167 NEW)

# 寻找依赖库
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# 手动指定Boost库的路径
set(Boost_INCLUDE_DIR /opt/homebrew/include)
set(Boost_LIBRARY_DIR /opt/homebrew/lib)

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

# 源文件
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 可执行文件
add_executable(team_task_manager ${SOURCES})

# 链接库
target_link_libraries(team_task_manager
    ${SQLite3_LIBRARIES}
    ${Boost_LIBRARY_DIR}/libboost_filesystem.dylib
    ${OPENSSL_LIBRARIES}
)

# 安装目标
install(TARGETS team_task_manager DESTINATION bin)

# 测试
enable_testing()
find_package(GTest REQUIRED)

file(GLOB_RECURSE TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*.cpp")
add_executable(tests ${TEST_SOURCES})

target_link_libraries(tests
    ${SQLite3_LIBRARIES}
    GTest::GTest
    GTest::Main
    ${Boost_LIBRARY_DIR}/libboost_filesystem.dylib
    ${OPENSSL_LIBRARIES}
)

# 包含源文件目录，以便测试目标可以访问所有源文件
# 排除main.cpp，因为测试目标不需要main函数
list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")
target_sources(tests PRIVATE ${SOURCES})

add_test(NAME all_tests COMMAND tests)

# 生成文档
option(BUILD_DOC "Build documentation" OFF)
if(BUILD_DOC)
    find_package(Doxygen REQUIRED)
    set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${PROJECT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()