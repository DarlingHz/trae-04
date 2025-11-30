cmake_minimum_required(VERSION 3.10)
project(OnlineJudgeBackend)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
add_compile_options(-Wall -Wextra -pedantic)

# 项目不需要Boost库

# 添加头文件目录
include_directories(include)
include_directories(src)
include_directories(${PROJECT_SOURCE_DIR}/src)

# 设置源代码目录
aux_source_directory(${PROJECT_SOURCE_DIR}/src/models MODEL_SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/controllers CONTROLLER_SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/auth AUTH_SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/common COMMON_SOURCES)
list(APPEND COMMON_SOURCES ${PROJECT_SOURCE_DIR}/src/common/json.cpp)

# 设置主程序入口文件
set(MAIN_SOURCE ${PROJECT_SOURCE_DIR}/src/main.cpp)

# 查找 OpenSSL 库
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

# 创建可执行文件
add_executable(online_judge_backend ${MAIN_SOURCE} ${MODEL_SOURCES} ${CONTROLLER_SOURCES} ${AUTH_SOURCES} ${COMMON_SOURCES})

# 设置编译选项
target_compile_options(online_judge_backend PRIVATE -Wall -Wextra -pedantic -std=c++17)

# 链接 OpenSSL 库
target_link_libraries(online_judge_backend ${OPENSSL_LIBRARIES})

# 链接 Boost 库
# Removed Boost library link as it's not needed

# 如果需要测试，添加测试目标
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
