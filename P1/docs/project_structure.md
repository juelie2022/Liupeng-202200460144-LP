# SM4加密算法优化实现项目结构

本文档介绍了SM4加密算法优化实现项目的目录结构和各个组件的功能。

## 目录结构

```
sm4-optimization/
├── include/                  # 头文件目录
│   ├── sm4.h                 # SM4基本API定义
│   ├── sm4_gcm.h             # SM4-GCM模式API定义
│   ├── sm4_internal.h        # 内部函数和数据结构定义
│   └── sm4_cpu_features.h    # CPU特性检测API
├── src/                      # 源代码目录
│   ├── basic/                # 基本实现
│   │   ├── sm4_basic.c       # 基本SM4实现
│   │   └── CMakeLists.txt    # 基本实现构建配置
│   ├── t_table/              # T表优化实现
│   │   ├── sm4_t_table.c     # T表SM4实现
│   │   └── CMakeLists.txt    # T表实现构建配置
│   ├── aesni/                # AES-NI优化实现
│   │   ├── sm4_aesni.c       # AES-NI SM4实现
│   │   └── CMakeLists.txt    # AES-NI实现构建配置
│   ├── modern/               # 现代指令集优化实现
│   │   ├── sm4_modern_inst.c # GFNI SM4实现
│   │   └── CMakeLists.txt    # 现代指令集实现构建配置
│   ├── gcm/                  # GCM模式实现
│   │   ├── sm4_gcm.c         # SM4-GCM实现
│   │   └── CMakeLists.txt    # GCM实现构建配置
│   ├── common/               # 公共代码
│   │   ├── sm4_common.c      # 公共函数实现
│   │   ├── sm4_cpu_features.c # CPU特性检测实现
│   │   └── CMakeLists.txt    # 公共代码构建配置
│   └── CMakeLists.txt        # 源代码构建配置
├── examples/                 # 示例代码
│   ├── basic_example/        # 基本使用示例
│   │   ├── sm4_basic_example.c # 基本示例源码
│   │   └── CMakeLists.txt    # 基本示例构建配置
│   ├── gcm_example/          # GCM模式示例
│   │   ├── sm4_gcm_example.c # GCM示例源码
│   │   └── CMakeLists.txt    # GCM示例构建配置
│   ├── benchmark/            # 性能测试示例
│   │   ├── sm4_benchmark.c   # 性能测试源码
│   │   └── CMakeLists.txt    # 性能测试构建配置
│   └── CMakeLists.txt        # 示例构建配置
├── test/                     # 测试代码
│   ├── unit/                 # 单元测试
│   │   ├── sm4_test.c        # 单元测试源码
│   │   └── CMakeLists.txt    # 单元测试构建配置
│   ├── benchmark/            # 性能测试
│   │   ├── sm4_benchmark_test.c # 性能测试源码
│   │   └── CMakeLists.txt    # 性能测试构建配置
│   └── CMakeLists.txt        # 测试构建配置
├── docs/                     # 文档
│   ├── usage_guide.md        # 使用指南
│   └── optimization_techniques.md # 优化技术详解
├── CMakeLists.txt            # 项目构建配置
├── build.sh                  # 构建脚本
├── install.sh                # 安装脚本
├── LICENSE                   # 许可证文件
└── README.md                 # 项目说明
```

## 组件说明

### 头文件 (include/)

- **sm4.h**: 定义SM4基本加解密API，包括上下文初始化、密钥设置和数据块加解密函数。
- **sm4_gcm.h**: 定义SM4-GCM模式API，包括一步式和分步式接口。
- **sm4_internal.h**: 定义内部使用的函数和数据结构，不对外暴露。
- **sm4_cpu_features.h**: 定义CPU特性检测API，用于运行时选择最佳实现。

### 源代码 (src/)

#### 基本实现 (basic/)

- **sm4_basic.c**: 实现基本的SM4算法，直接按照标准文档实现，不包含任何优化。

#### T表优化实现 (t_table/)

- **sm4_t_table.c**: 使用预计算的T表优化SM4实现，提高性能。

#### AES-NI优化实现 (aesni/)

- **sm4_aesni.c**: 利用AES-NI指令集优化SM4实现，在支持AES-NI的处理器上提供更高性能。

#### 现代指令集优化实现 (modern/)

- **sm4_modern_inst.c**: 利用GFNI等现代指令集优化SM4实现，在最新处理器上提供最高性能。

#### GCM模式实现 (gcm/)

- **sm4_gcm.c**: 实现SM4-GCM认证加密模式，提供数据加密和完整性保护。

#### 公共代码 (common/)

- **sm4_common.c**: 实现各实现共用的函数和数据结构。
- **sm4_cpu_features.c**: 实现CPU特性检测功能，用于选择最佳实现。

### 示例 (examples/)

- **basic_example/**: 演示SM4基本加解密功能的使用。
- **gcm_example/**: 演示SM4-GCM模式的使用。
- **benchmark/**: 提供性能测试示例。

### 测试 (test/)

- **unit/**: 包含单元测试，验证各实现的正确性。
- **benchmark/**: 包含性能测试，比较各实现的性能。

### 文档 (docs/)

- **usage_guide.md**: 提供详细的使用指南和示例代码。
- **optimization_techniques.md**: 详细介绍SM4优化技术。

### 构建和安装

- **CMakeLists.txt**: 主项目构建配置。
- **build.sh**: 简化构建过程的脚本。
- **install.sh**: 简化安装过程的脚本。

## 库依赖关系

```
sm4_gcm
  ├── sm4_basic
  ├── sm4_t_table
  ├── sm4_aesni (可选，取决于CPU支持)
  └── sm4_modern_inst (可选，取决于CPU支持)
```

## 编译时配置选项

- **BUILD_TESTS**: 是否构建测试程序
- **BUILD_BENCHMARKS**: 是否构建基准测试程序
- **ENABLE_AESNI**: 是否启用AES-NI优化
- **ENABLE_GFNI**: 是否启用GFNI优化

## 运行时行为

1. 程序启动时，通过`sm4_get_cpu_features()`检测CPU特性
2. 根据检测结果，自动选择最佳SM4实现
3. 用户可以通过`sm4_get_best_implementation()`获取当前使用的实现名称