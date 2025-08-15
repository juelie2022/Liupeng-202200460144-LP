# SM4加密算法优化实现使用指南

本文档提供了SM4加密算法优化实现的快速入门指南和常见用例。

## 快速开始

### 构建和安装

1. 克隆仓库：

```bash
git clone https://github.com/yourusername/sm4-optimization.git
cd sm4-optimization
```

2. 使用构建脚本构建项目：

```bash
./build.sh
```

3. 安装库（可选）：

```bash
./install.sh
```

### 运行示例

构建完成后，可以运行以下示例程序：

```bash
# 基本示例
./build/examples/basic_example/sm4_basic_example

# GCM模式示例
./build/examples/gcm_example/sm4_gcm_example

# 性能基准测试
./build/examples/benchmark/sm4_benchmark
```

## 常见用例

### 1. 单个数据块加密

```c
#include "sm4.h"
#include <stdio.h>

int main() {
    SM4_Context ctx;
    uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                       0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t plaintext[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t ciphertext[16];
    
    // 初始化加密上下文
    sm4_set_encrypt_key(&ctx, key);
    
    // 加密单个数据块
    sm4_encrypt_block(&ctx, ciphertext, plaintext);
    
    // 打印结果
    printf("密文: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");
    
    return 0;
}
```

### 2. 多个数据块加密（ECB模式）

```c
#include "sm4.h"
#include <stdio.h>

int main() {
    SM4_Context ctx;
    uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                       0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t plaintext[64]; // 4个数据块
    uint8_t ciphertext[64];
    
    // 初始化测试数据
    for (int i = 0; i < 64; i++) {
        plaintext[i] = i;
    }
    
    // 初始化加密上下文
    sm4_set_encrypt_key(&ctx, key);
    
    // 加密多个数据块（ECB模式）
    for (int i = 0; i < 4; i++) {
        sm4_encrypt_block(&ctx, ciphertext + i * 16, plaintext + i * 16);
    }
    
    return 0;
}
```

### 3. 使用GCM模式进行认证加密

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
    
    const char *message = "这是一条需要加密和认证的消息";
    size_t message_len = strlen(message);
    
    uint8_t *ciphertext = malloc(message_len);
    uint8_t *decrypted = malloc(message_len);
    uint8_t tag[16];
    
    // 加密和认证
    sm4_gcm_encrypt_and_tag(
        key, iv, sizeof(iv),
        aad, sizeof(aad),
        (const uint8_t *)message, message_len,
        ciphertext, tag, sizeof(tag)
    );
    
    // 解密和验证
    int result = sm4_gcm_decrypt_and_verify(
        key, iv, sizeof(iv),
        aad, sizeof(aad),
        ciphertext, message_len,
        tag, sizeof(tag),
        decrypted
    );
    
    if (result == 0) {
        printf("认证成功!\n");
        printf("解密消息: %.*s\n", (int)message_len, decrypted);
    } else {
        printf("认证失败!\n");
    }
    
    free(ciphertext);
    free(decrypted);
    
    return 0;
}
```

### 4. 分步式GCM操作

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
    
    const char *message = "这是一条需要加密和认证的消息";
    size_t message_len = strlen(message);
    
    uint8_t *ciphertext = malloc(message_len);
    uint8_t *decrypted = malloc(message_len);
    uint8_t tag[16];
    uint8_t verify_tag[16];
    
    // 初始化GCM上下文
    SM4_GCM_Context encrypt_ctx;
    SM4_GCM_Context decrypt_ctx;
    
    // 加密过程
    sm4_gcm_init(&encrypt_ctx, key, iv, sizeof(iv));
    sm4_gcm_aad(&encrypt_ctx, aad, sizeof(aad));
    sm4_gcm_encrypt(&encrypt_ctx, ciphertext, (const uint8_t *)message, message_len);
    sm4_gcm_finish(&encrypt_ctx, tag, sizeof(tag));
    
    // 解密过程
    sm4_gcm_init(&decrypt_ctx, key, iv, sizeof(iv));
    sm4_gcm_aad(&decrypt_ctx, aad, sizeof(aad));
    sm4_gcm_decrypt(&decrypt_ctx, decrypted, ciphertext, message_len);
    sm4_gcm_finish(&decrypt_ctx, verify_tag, sizeof(verify_tag));
    
    // 验证标签
    if (memcmp(tag, verify_tag, sizeof(tag)) == 0) {
        printf("认证成功!\n");
        printf("解密消息: %.*s\n", (int)message_len, decrypted);
    } else {
        printf("认证失败!\n");
    }
    
    free(ciphertext);
    free(decrypted);
    
    return 0;
}
```

### 5. 检测CPU特性并选择最佳实现

```c
#include "sm4.h"
#include "sm4_cpu_features.h"
#include <stdio.h>

int main() {
    SM4_CPU_Features features = sm4_get_cpu_features();
    
    printf("CPU特性检测:\n");
    printf("  SSE2: %s\n", features.has_sse2 ? "支持" : "不支持");
    printf("  AES-NI: %s\n", features.has_aesni ? "支持" : "不支持");
    printf("  AVX: %s\n", features.has_avx ? "支持" : "不支持");
    printf("  AVX2: %s\n", features.has_avx2 ? "支持" : "不支持");
    printf("  AVX-512F: %s\n", features.has_avx512f ? "支持" : "不支持");
    printf("  GFNI: %s\n", features.has_gfni ? "支持" : "不支持");
    printf("  VAES: %s\n", features.has_vaes ? "支持" : "不支持");
    printf("  VPCLMULQDQ: %s\n", features.has_vpclmulqdq ? "支持" : "不支持");
    
    printf("\n最佳SM4实现: %s\n", sm4_get_best_implementation());
    
    return 0;
}
```

## 编译和链接

### 使用CMake

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

# 查找SM4库
find_package(SM4_Optimization REQUIRED)

# 添加可执行文件
add_executable(my_app main.c)

# 链接SM4库
target_link_libraries(my_app sm4_basic sm4_gcm)
```

### 使用GCC

```bash
gcc -o my_app main.c -I/usr/local/include/sm4_opt -L/usr/local/lib -lsm4_basic -lsm4_gcm
```

## 性能优化建议

1. 对于大量数据，尽量批量处理，减少函数调用开销
2. 在支持AES-NI或GFNI的平台上，确保启用相应的优化
3. 对于安全敏感应用，注意防止侧信道攻击
4. 使用GCM模式时，可以考虑重用IV，但需确保IV不重复
5. 在多线程环境中，每个线程应使用独立的上下文对象