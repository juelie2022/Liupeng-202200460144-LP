cmake_minimum_required(VERSION 3.10)
project(waterprint)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenCV包
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 包含头文件目录
include_directories(include)

# 添加源文件
set(SOURCES
    src/main.cpp
    src/watermark.cpp
    src/utils.cpp
)

# 生成可执行文件
add_executable(waterprint ${SOURCES})

# 链接OpenCV库
target_link_libraries(waterprint ${OpenCV_LIBS})

# 添加测试可执行文件
add_executable(test_waterprint test/test_main.cpp src/watermark.cpp src/utils.cpp)
target_link_libraries(test_waterprint ${OpenCV_LIBS})

# 安装目标
install(TARGETS waterprint DESTINATION bin)