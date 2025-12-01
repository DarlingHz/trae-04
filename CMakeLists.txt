cmake_minimum_required(VERSION 3.10)
project(PersonalKnowledgeCards)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)



# 添加头文件目录
include_directories(include)
include_directories(src)

# 查找第三方库
# 移除SQLiteCpp依赖项，将直接使用SQLite3库
find_package(SQLite3 REQUIRED)

# 设置nlohmann_json的查找路径
find_package(nlohmann_json)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, will download and build")
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# 从GitHub下载httplib的源代码，并将其构建为一个静态库
include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.1
)
FetchContent_MakeAvailable(httplib)

# 移除jwt-cpp依赖项，将使用自定义JWT实现

# 移除bcrypt依赖项，将使用自定义密码哈希实现

# 收集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 创建可执行文件
add_executable(pkc_server ${SOURCES})

# 链接第三方库
target_link_libraries(pkc_server PRIVATE
    sqlite3
    nlohmann_json::nlohmann_json
    httplib::httplib
)

# 禁用httplib的所有与CoreFoundation相关的功能，从而避免使用CoreFoundation框架
target_compile_definitions(pkc_server PRIVATE CPPHTTPLIB_NO_MACOS)

# 安装目标
install(TARGETS pkc_server DESTINATION bin)
install(FILES config/config.json DESTINATION config)

# 测试（暂时移除，稍后添加）
# enable_testing()
# file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
# add_executable(pkc_test ${TEST_SOURCES})
# target_link_libraries(pkc_test
#     SQLite3::SQLite3
#     nlohmann_json::nlohmann_json
#     httplib::httplib
#     gtest
#     gtest_main
# )
# add_test(NAME pkc_test COMMAND pkc_test)
