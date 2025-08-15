# SM3软件实现与优化项目

## 项目概述
本项目实现了SM3哈希算法的软件实现与优化，包括基本实现、性能优化、长度扩展攻击验证、以及基于RFC6962的Merkle树构建。

## 功能特性
- **SM3基本实现**: 完整的SM3哈希算法实现
- **性能优化**: 多层次的软件优化策略
- **长度扩展攻击验证**: 验证SM3对长度扩展攻击的防护
- **Merkle树构建**: 支持10万叶子节点的Merkle树
- **存在性证明**: 构建叶子的存在性证明
- **不存在性证明**: 构建叶子的不存在性证明

## 项目结构
```
P4/
├── README.md              # 项目说明文档
├── Makefile               # 编译配置文件
├── include/               # 头文件目录
│   ├── sm3.h             # SM3算法头文件
│   ├── merkle.h          # Merkle树头文件
│   └── utils.h           # 工具函数头文件
├── src/                   # 源代码目录
│   ├── sm3.c             # SM3算法实现
│   ├── sm3_optimized.c   # SM3优化版本
│   ├── merkle.c          # Merkle树实现
│   └── utils.c           # 工具函数实现
├── test/                  # 测试文件目录
│   ├── test_sm3.c        # SM3测试
│   ├── test_length_extension.c  # 长度扩展攻击测试
│   └── test_merkle.c     # Merkle树测试
└── docs/                  # 文档目录
    ├── sm3_optimization.md  # 优化策略文档
    └── merkle_proofs.md     # 证明构建文档
```

## 编译说明
```bash
# 编译所有目标
make all

# 编译特定目标
make sm3_test
make length_extension_test
make merkle_test

# 清理编译文件
make clean

# 运行性能测试
make benchmark
```

## 依赖要求
- GCC 7.0+ 或 Clang 5.0+
- Make 3.8+
- 支持C99标准

## 使用示例
```bash
# 编译项目
make all

# 运行SM3基本测试
./test/sm3_test

# 运行长度扩展攻击测试
./test/length_extension_test

# 运行Merkle树测试
./test/merkle_test
```

## 性能优化策略
1. **循环展开**: 减少循环开销
2. **查表优化**: 使用预计算表加速运算
3. **SIMD优化**: 利用向量指令并行处理
4. **内存对齐**: 优化内存访问模式
5. **编译器优化**: 启用高级优化选项

## 作者
基于付勇老师的PPT和RFC6962标准实现

## 许可证
MIT License
