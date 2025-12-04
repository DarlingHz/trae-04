cmake_minimum_required(VERSION 3.16)
project(ExchangeEngine VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 查找依赖
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

# 检查是否安装了Crow框架
# Crow是头文件库，需要手动下载或通过git submodule添加# 设置Crow框架的路径
set(CROW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/crow/include")

# 使用Homebrew安装的ASIO
set(ASIO_INCLUDE_DIR "/opt/homebrew/Cellar/asio/1.36.0/include")
include_directories(${ASIO_INCLUDE_DIR})

if(NOT EXISTS ${CROW_INCLUDE_DIR}) 
    message(FATAL_ERROR "Crow framework not found. Please clone it into third_party/crow:")
    message("git clone --recursive https://github.com/CrowCpp/Crow.git ${CMAKE_SOURCE_DIR}/third_party/crow")
endif()

# 添加头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CROW_INCLUDE_DIR}
    ${SQLite3_INCLUDE_DIRS}
)

# 收集源文件
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# 创建可执行文件
add_executable(exchange_engine ${SOURCES})

# 链接依赖
target_link_libraries(exchange_engine
    PRIVATE
        Threads::Threads
        ${SQLite3_LIBRARIES}
)

# 设置输出目录
set_target_properties(exchange_engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin
)

# 添加测试目录
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
