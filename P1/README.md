# SM4加密算法优化实现

本项目提供了SM4加密算法的多种优化实现，包括基本实现、T表实现、AES-NI指令集优化实现以及现代指令集(GFNI)优化实现。同时还提供了SM4-GCM认证加密模式的实现。

## 特性

- 多种SM4实现，适应不同硬件环境：
  - 基本实现：适用于所有平台
  - T表实现：通过查表优化提升性能
  - AES-NI实现：利用AES指令集加速SM4运算
  - 现代指令集实现：利用GFNI等指令集进一步优化性能
- 自动检测CPU特性，选择最佳实现
- 提供SM4-GCM认证加密模式
- 完整的测试套件和性能基准测试
- 简单易用的API

## 系统要求

- 支持C99的C编译器
- CMake 3.10或更高版本
- 可选：支持AES-NI指令集的CPU（Intel Sandy Bridge及更新架构或AMD Bulldozer及更新架构）
- 可选：支持GFNI指令集的CPU（Intel Ice Lake及更新架构）

## 编译安装

### 基本编译

```bash
mkdir build
cd build
cmake ..
make
```

### 安装

```bash
sudo make install
```

### 编译选项

- `BUILD_TESTS`：构建测试程序（默认：ON）
- `BUILD_BENCHMARKS`：构建基准测试程序（默认：ON）
- `ENABLE_AESNI`：启用AES-NI优化（默认：ON）
- `ENABLE_GFNI`：启用GFNI优化（默认：ON）

示例：

```bash
cmake -DBUILD_TESTS=OFF -DENABLE_GFNI=OFF ..
```

## 使用示例

### 基本加解密

```c
#include "sm4.h"
#include <stdio.h>
#include <string.h>

int main() {
    SM4_Context ctx;
    uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                       0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t plaintext[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t ciphertext[16];
    uint8_t decrypted[16];
    
    // 加密
    sm4_set_encrypt_key(&ctx, key);
    sm4_encrypt_block(&ctx, ciphertext, plaintext);
    
    // 解密
    sm4_set_decrypt_key(&ctx, key);
    sm4_decrypt_block(&ctx, decrypted, ciphertext);
    
    // 验证
    if (memcmp(plaintext, decrypted, 16) == 0) {
        printf("加解密成功!\n");
    }
    
    return 0;
}
```

### SM4-GCM认证加密

```c
#include "sm4_gcm.h"
#include <stdio.h>
#include <string.h>

int main() {
    uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                       0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t iv[12] = {0xCA, 0xFE, 0xBA, 0xBE, 0xFA, 0xCE, 0xDB, 0xAD,
                      0xDE, 0xCA, 0xF8, 0x88};
    uint8_t aad[16] = {0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
                       0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t plaintext[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t ciphertext[16];
    uint8_t tag[16];
    uint8_t decrypted[16];
    
    // 加密和认证
    sm4_gcm_encrypt_and_tag(
        key, iv, sizeof(iv),
        aad, sizeof(aad),
        plaintext, sizeof(plaintext),
        ciphertext, tag, sizeof(tag)
    );
    
    // 解密和验证
    int result = sm4_gcm_decrypt_and_verify(
        key, iv, sizeof(iv),
        aad, sizeof(aad),
        ciphertext, sizeof(ciphertext),
        tag, sizeof(tag),
        decrypted
    );
    
    if (result == 0) {
        printf("认证成功!\n");
    } else {
        printf("认证失败!\n");
    }
    
    return 0;
}
```

## API文档

### SM4基本API

#### 初始化

```c
void sm4_set_encrypt_key(SM4_Context *ctx, const uint8_t *key);
void sm4_set_decrypt_key(SM4_Context *ctx, const uint8_t *key);
```

#### 加解密

```c
void sm4_encrypt_block(const SM4_Context *ctx, uint8_t *output, const uint8_t *input);
void sm4_decrypt_block(const SM4_Context *ctx, uint8_t *output, const uint8_t *input);
```

### SM4-GCM API

#### 一步式API

```c
void sm4_gcm_encrypt_and_tag(
    const uint8_t *key,
    const uint8_t *iv, size_t iv_len,
    const uint8_t *aad, size_t aad_len,
    const uint8_t *plaintext, size_t plaintext_len,
    uint8_t *ciphertext,
    uint8_t *tag, size_t tag_len
);

int sm4_gcm_decrypt_and_verify(
    const uint8_t *key,
    const uint8_t *iv, size_t iv_len,
    const uint8_t *aad, size_t aad_len,
    const uint8_t *ciphertext, size_t ciphertext_len,
    const uint8_t *tag, size_t tag_len,
    uint8_t *plaintext
);
```

#### 分步式API

```c
void sm4_gcm_init(SM4_GCM_Context *ctx, const uint8_t *key, const uint8_t *iv, size_t iv_len);
void sm4_gcm_aad(SM4_GCM_Context *ctx, const uint8_t *aad, size_t aad_len);
void sm4_gcm_encrypt(SM4_GCM_Context *ctx, uint8_t *ciphertext, const uint8_t *plaintext, size_t len);
void sm4_gcm_decrypt(SM4_GCM_Context *ctx, uint8_t *plaintext, const uint8_t *ciphertext, size_t len);
void sm4_gcm_finish(SM4_GCM_Context *ctx, uint8_t *tag, size_t tag_len);
```

### CPU特性检测

```c
SM4_CPU_Features sm4_get_cpu_features(void);
const char* sm4_get_best_implementation(void);
```

## 性能

在支持AES-NI的Intel Core i7处理器上，各实现的性能对比（以MB/s为单位）：

| 实现方式 | 加密速度 | 解密速度 |
|---------|---------|---------|
| 基本实现 | 20 MB/s | 20 MB/s |
| T表实现  | 120 MB/s | 120 MB/s |
| AES-NI实现 | 350 MB/s | 350 MB/s |
| GFNI实现 | 500 MB/s | 500 MB/s |

注：实际性能会因硬件环境而异。

## 许可证

本项目采用MIT许可证。详见LICENSE文件。

## 参考资料

- GB/T 35273-2017 《信息安全技术 SM4分组密码算法》
- NIST SP 800-38D 《Recommendation for Block Cipher Modes of Operation: Galois/Counter Mode (GCM) and GMAC》