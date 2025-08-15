#!/bin/bash

# SM4加密算法优化实现构建和测试脚本

set -e

# 显示帮助信息
show_help() {
    echo "SM4加密算法优化实现构建和测试脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help            显示此帮助信息"
    echo "  -c, --clean           清理构建目录"
    echo "  -r, --release         构建发布版本（优化级别：O3）"
    echo "  -d, --debug           构建调试版本（包含调试信息）"
    echo "  -t, --test            运行测试"
    echo "  -b, --benchmark       运行基准测试"
    echo "  --no-aesni            禁用AES-NI优化"
    echo "  --no-gfni             禁用GFNI优化"
    echo ""
}

# 默认选项
BUILD_TYPE="Release"
RUN_TESTS=0
RUN_BENCHMARK=0
CLEAN_BUILD=0
ENABLE_AESNI="ON"
ENABLE_GFNI="ON"

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=1
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -t|--test)
            RUN_TESTS=1
            shift
            ;;
        -b|--benchmark)
            RUN_BENCHMARK=1
            shift
            ;;
        --no-aesni)
            ENABLE_AESNI="OFF"
            shift
            ;;
        --no-gfni)
            ENABLE_GFNI="OFF"
            shift
            ;;
        *)
            echo "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
done

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# 清理构建目录
if [ $CLEAN_BUILD -eq 1 ]; then
    echo "清理构建目录..."
    rm -rf "${BUILD_DIR}"
fi

# 创建构建目录
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# 配置项目
echo "配置项目..."
cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
      -DENABLE_AESNI="${ENABLE_AESNI}" \
      -DENABLE_GFNI="${ENABLE_GFNI}" \
      "${PROJECT_ROOT}"

# 构建项目
echo "构建项目..."
cmake --build . -- -j$(nproc)

# 运行测试
if [ $RUN_TESTS -eq 1 ]; then
    echo "运行测试..."
    ctest --output-on-failure
fi

# 运行基准测试
if [ $RUN_BENCHMARK -eq 1 ]; then
    echo "运行基准测试..."
    ./test/benchmark/sm4_benchmark_test
fi

echo "构建完成！"
echo "可执行文件位于: ${BUILD_DIR}/examples"