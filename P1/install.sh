#!/bin/bash

# SM4加密算法优化实现安装脚本

set -e

# 显示帮助信息
show_help() {
    echo "SM4加密算法优化实现安装脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help            显示此帮助信息"
    echo "  -p, --prefix=DIR      安装到指定目录（默认：/usr/local）"
    echo "  --no-aesni            禁用AES-NI优化"
    echo "  --no-gfni             禁用GFNI优化"
    echo ""
}

# 默认选项
INSTALL_PREFIX="/usr/local"
ENABLE_AESNI="ON"
ENABLE_GFNI="ON"

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -p|--prefix=*)
            if [[ "$1" == *"="* ]]; then
                INSTALL_PREFIX="${1#*=}"
            else
                INSTALL_PREFIX="$2"
                shift
            fi
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

# 创建构建目录
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# 配置项目
echo "配置项目..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
      -DENABLE_AESNI="${ENABLE_AESNI}" \
      -DENABLE_GFNI="${ENABLE_GFNI}" \
      "${PROJECT_ROOT}"

# 构建项目
echo "构建项目..."
cmake --build . -- -j$(nproc)

# 安装项目
echo "安装项目到 ${INSTALL_PREFIX}..."
if [ "$INSTALL_PREFIX" != "/usr/local" ]; then
    cmake --install .
else
    echo "需要管理员权限安装到 ${INSTALL_PREFIX}..."
    sudo cmake --install .
fi

echo "安装完成！"
echo "库文件位于: ${INSTALL_PREFIX}/lib"
echo "头文件位于: ${INSTALL_PREFIX}/include/sm4_opt"