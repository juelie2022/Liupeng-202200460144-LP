# Project 5: SM2软件实现优化

## 项目概述
本项目实现了SM2椭圆曲线密码算法的Python版本，包括基础实现、算法改进、误用验证和伪造中本聪签名的POC。

## 项目结构
```
P5/
├── README.md                 # 项目说明文档
├── requirements.txt          # Python依赖包
├── sm2/                     # SM2核心实现
│   ├── __init__.py
│   ├── core.py              # 基础SM2实现
│   ├── optimized.py         # 优化算法实现
│   └── utils.py             # 工具函数
├── poc/                     # 概念验证代码
│   ├── __init__.py
│   ├── signature_misuse.py  # 签名算法误用验证
│   └── satoshi_forgery.py   # 伪造中本聪签名
├── docs/                    # 文档
│   ├── sm2_implementation.md # SM2实现文档
│   ├── algorithm_analysis.md # 算法分析文档
│   └── vulnerability_report.md # 漏洞分析报告
└── tests/                   # 测试代码
    ├── __init__.py
    ├── test_sm2_core.py
    ├── test_optimizations.py
    └── test_vulnerabilities.py
```

## 功能特性
1. **SM2基础实现**: 完整的SM2椭圆曲线密码算法实现
2. **算法优化**: 多种优化策略的性能对比
3. **误用验证**: 基于PDF文档的签名算法误用POC
4. **伪造签名**: 中本聪数字签名伪造的演示

## 安装和运行
```bash
pip install -r requirements.txt
python -m pytest tests/
```

## 使用示例
```python
from sm2.core import SM2
from sm2.optimized import SM2Optimized

# 基础SM2使用
sm2 = SM2()
public_key, private_key = sm2.generate_keypair()
signature = sm2.sign(message, private_key)
is_valid = sm2.verify(message, signature, public_key)

# 优化版本使用
sm2_opt = SM2Optimized()
# ... 使用优化版本
```

## 注意事项
- 本项目仅用于学习和研究目的
- 请勿在生产环境中使用
- 所有漏洞演示仅用于安全研究
