cmake_minimum_required(VERSION 3.10)
project(SM4_Optimization VERSION 1.0.0 LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
option(BUILD_TESTS "Build test programs" ON)
option(BUILD_BENCHMARKS "Build benchmark programs" ON)
option(ENABLE_AESNI "Enable AES-NI optimization" ON)
option(ENABLE_GFNI "Enable GFNI optimization" ON)

# 检查CPU特性
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
#include <immintrin.h>
int main() {
    __m128i a = _mm_setzero_si128();
    __m128i b = _mm_aesenc_si128(a, a);
    return 0;
}
" HAVE_AESNI)

check_cxx_source_compiles("
#include <immintrin.h>
int main() {
    __m128i a = _mm_setzero_si128();
    __m128i b = _mm_gf2p8affine_epi64_epi8(a, a, 0);
    return 0;
}
" HAVE_GFNI)

# 设置编译标志
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
    if(HAVE_AESNI AND ENABLE_AESNI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maes")
    endif()
    if(HAVE_GFNI AND ENABLE_GFNI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mgfni -mavx512vl -mavx512bw -mavx512dq")
    endif()
endif()

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 添加子目录
add_subdirectory(src)

# 构建测试
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# 构建示例
add_subdirectory(examples)

# 安装规则
install(DIRECTORY include/ DESTINATION include/sm4_opt)