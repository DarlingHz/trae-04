# 查找Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 创建测试可执行文件
add_executable(test_order_book test_order_book.cpp)

# 链接依赖
target_link_libraries(test_order_book
    PRIVATE
        ${GTEST_BOTH_LIBRARIES}
        Threads::Threads
        SQLite3::SQLite3
)

# 添加测试
add_test(NAME OrderBookTest COMMAND test_order_book)

# 设置输出目录
set_target_properties(test_order_book PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin
)
