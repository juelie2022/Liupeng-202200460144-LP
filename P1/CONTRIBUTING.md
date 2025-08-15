# 贡献指南

感谢您对SM4加密算法优化实现项目的关注！我们欢迎各种形式的贡献，包括但不限于代码改进、文档完善、bug修复和新功能实现。

## 贡献流程

1. Fork本仓库
2. 创建您的特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交您的更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建一个Pull Request

## 开发环境设置

1. 克隆仓库：

```bash
git clone https://github.com/yourusername/sm4-optimization.git
cd sm4-optimization
```

2. 创建构建目录：

```bash
mkdir build
cd build
```

3. 配置项目：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

4. 构建项目：

```bash
make
```

5. 运行测试：

```bash
ctest --output-on-failure
```

## 代码风格

我们遵循以下代码风格规范：

- 使用4个空格进行缩进，不使用制表符
- 函数和变量名使用小写字母和下划线（snake_case）
- 宏和常量使用大写字母和下划线
- 每行不超过100个字符
- 函数应该有清晰的文档注释，说明其功能、参数和返回值
- 避免使用全局变量
- 所有公共API都应该在头文件中有文档注释

示例：

```c
/**
 * 设置SM4加密密钥
 *
 * @param ctx 指向SM4上下文的指针
 * @param key 16字节的密钥
 */
void sm4_set_encrypt_key(SM4_Context *ctx, const uint8_t *key);
```

## 提交消息规范

提交消息应该清晰地描述更改内容，并遵循以下格式：

```
<类型>(<范围>): <描述>

[可选的详细描述]

[可选的脚注]
```

类型可以是：
- feat: 新功能
- fix: bug修复
- docs: 文档更改
- style: 不影响代码含义的更改（空格、格式化等）
- refactor: 既不修复bug也不添加功能的代码更改
- perf: 提高性能的代码更改
- test: 添加或修正测试
- build: 影响构建系统或外部依赖的更改
- ci: 更改CI配置文件和脚本

示例：

```
feat(aesni): 添加AES-NI指令集优化实现

使用AES-NI指令集优化SM4加密算法，提高在支持AES-NI的处理器上的性能。

性能测试显示比基本实现提升约5倍。
```

## 测试要求

- 所有新功能必须包含相应的单元测试
- 所有bug修复必须包含能重现问题的测试
- 测试覆盖率应保持在80%以上
- 所有测试必须通过才能合并代码

## 性能基准测试

对于性能优化，请提供基准测试结果，包括：

1. 测试环境（CPU型号、内存大小、操作系统等）
2. 测试方法（数据大小、迭代次数等）
3. 优化前后的性能对比
4. 与其他实现的性能对比（如适用）

## 安全考虑

- 所有密码学实现必须遵循相关标准
- 避免引入侧信道漏洞
- 对于安全敏感的更改，请详细说明安全影响
- 不要在代码中硬编码密钥或其他敏感信息

## 文档要求

- 所有新功能必须有相应的文档
- 更新README.md以反映重要更改
- 保持API文档的最新状态
- 提供示例代码展示如何使用新功能

## 许可证

通过向本项目贡献代码，您同意您的贡献将在MIT许可证下发布。

## 联系方式

如有任何问题，请通过以下方式联系我们：

- 提交Issue
- 发送邮件至：example@example.com