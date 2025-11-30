cmake_minimum_required(VERSION 3.10)
project(InventoryOrderManagement)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找依赖库
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)

# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/main.cpp
    src/server.cpp
    src/database.cpp
    src/product_service.cpp
    src/order_service.cpp
    src/stats_service.cpp
)

# 可执行文件
add_executable(inventory_order_management ${SOURCES})

# 链接依赖库
target_link_libraries(inventory_order_management
    ${SQLite3_LIBRARIES}
    ${CURL_LIBRARIES}
    pthread
)

# 测试
enable_testing()
add_subdirectory(tests)