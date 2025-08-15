#include "../include/sm3.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 演示长度扩展攻击
static void demonstrate_length_extension_attack() {
    printf("=== 长度扩展攻击演示 ===\n\n");
    
    // 场景：假设我们知道一个秘密消息的哈希值，但不知道消息内容
    const char *secret_message = "secret_key_12345";
    const char *extension = "&admin=true&role=superuser";
    
    printf("攻击场景:\n");
    printf("  已知: 秘密消息的哈希值\n");
    printf("  目标: 构造一个包含扩展数据的有效哈希值\n");
    printf("  方法: 利用哈希函数的内部状态进行长度扩展\n\n");
    
    // 步骤1: 计算原始消息的哈希
    uint8_t original_digest[SM3_DIGEST_SIZE];
    sm3_hash_string(secret_message, original_digest);
    
    printf("步骤1: 计算原始消息哈希\n");
    printf("  原始消息: \"%s\"\n", secret_message);
    printf("  原始哈希: ");
    sm3_print_digest(original_digest);
    printf("  消息长度: %zu 字节\n\n", strlen(secret_message));
    
    // 步骤2: 尝试长度扩展攻击
    uint8_t attack_digest[SM3_DIGEST_SIZE];
    int attack_success = sm3_length_extension_attack(
        original_digest,
        strlen(secret_message),
        (const uint8_t*)extension,
        strlen(extension),
        attack_digest
    );
    
    printf("步骤2: 执行长度扩展攻击\n");
    printf("  扩展数据: \"%s\"\n", extension);
    printf("  攻击结果: %s\n", attack_success ? "成功" : "失败");
    
    if (attack_success) {
        printf("  攻击生成的哈希: ");
        sm3_print_digest(attack_digest);
        printf("\n");
        
        // 步骤3: 验证攻击结果
        printf("步骤3: 验证攻击结果\n");
        
        // 构造完整的消息（原始消息 + 扩展数据）
        char full_message[512];
        snprintf(full_message, sizeof(full_message), "%s%s", secret_message, extension);
        
        // 计算真实哈希
        uint8_t legitimate_digest[SM3_DIGEST_SIZE];
        sm3_hash_string(full_message, legitimate_digest);
        
        printf("  完整消息: \"%s\"\n", full_message);
        printf("  真实哈希: ");
        sm3_print_digest(legitimate_digest);
        
        // 比较结果
        if (memcmp(attack_digest, legitimate_digest, SM3_DIGEST_SIZE) == 0) {
            printf("  ✓ 攻击成功！攻击生成的哈希与真实哈希匹配\n");
        } else {
            printf("  ✗ 攻击失败！攻击生成的哈希与真实哈希不匹配\n");
        }
        
        printf("\n攻击分析:\n");
        printf("  1. 攻击者成功预测了哈希函数的内部状态\n");
        printf("  2. 利用内部状态计算了扩展数据的哈希值\n");
        printf("  3. 构造了一个有效的哈希值，无需知道原始消息\n");
        printf("  4. 这证明了SM3对长度扩展攻击的脆弱性\n");
        
    } else {
        printf("  攻击失败，无法执行长度扩展攻击\n");
    }
    
    printf("\n");
}

// 测试不同的消息长度
static void test_different_lengths() {
    printf("=== 测试不同消息长度的长度扩展攻击 ===\n\n");
    
    const char *extensions[] = {
        "short",
        "medium_length_extension",
        "very_long_extension_data_for_testing_purposes"
    };
    
    const char *base_messages[] = {
        "a",
        "short_msg",
        "medium_length_message_for_testing"
    };
    
    for (int i = 0; i < 3; i++) {
        const char *base_msg = base_messages[i];
        const char *ext = extensions[i];
        
        printf("测试 %d: 基础消息=\"%s\", 扩展=\"%s\"\n", 
               i + 1, base_msg, ext);
        
        uint8_t base_digest[SM3_DIGEST_SIZE];
        sm3_hash_string(base_msg, base_digest);
        
        uint8_t attack_digest[SM3_DIGEST_SIZE];
        int success = sm3_length_extension_attack(
            base_digest,
            strlen(base_msg),
            (const uint8_t*)ext,
            strlen(ext),
            attack_digest
        );
        
        if (success) {
            // 验证结果
            char full_msg[256];
            snprintf(full_msg, sizeof(full_msg), "%s%s", base_msg, ext);
            
            uint8_t real_digest[SM3_DIGEST_SIZE];
            sm3_hash_string(full_msg, real_digest);
            
            if (memcmp(attack_digest, real_digest, SM3_DIGEST_SIZE) == 0) {
                printf("  ✓ 攻击成功\n");
            } else {
                printf("  ✗ 攻击失败\n");
            }
        } else {
            printf("  ✗ 无法执行攻击\n");
        }
        printf("\n");
    }
}

// 测试边界情况
static void test_edge_cases() {
    printf("=== 测试边界情况 ===\n\n");
    
    // 测试空扩展
    printf("测试空扩展:\n");
    const char *msg = "test_message";
    uint8_t digest[SM3_DIGEST_SIZE];
    sm3_hash_string(msg, digest);
    
    uint8_t attack_digest[SM3_DIGEST_SIZE];
    int success = sm3_length_extension_attack(
        digest, strlen(msg), (const uint8_t*)"", 0, attack_digest
    );
    
    if (success) {
        printf("  ✓ 空扩展攻击成功\n");
        if (memcmp(digest, attack_digest, SM3_DIGEST_SIZE) == 0) {
            printf("  ✓ 结果正确（空扩展应该产生相同哈希）\n");
        } else {
            printf("  ✗ 结果错误\n");
        }
    } else {
        printf("  ✗ 空扩展攻击失败\n");
    }
    printf("\n");
    
    // 测试单字节扩展
    printf("测试单字节扩展:\n");
    success = sm3_length_extension_attack(
        digest, strlen(msg), (const uint8_t*)"x", 1, attack_digest
    );
    
    if (success) {
        printf("  ✓ 单字节扩展攻击成功\n");
        
        // 验证
        char full_msg[256];
        snprintf(full_msg, sizeof(full_msg), "%sx", msg);
        uint8_t real_digest[SM3_DIGEST_SIZE];
        sm3_hash_string(full_msg, real_digest);
        
        if (memcmp(attack_digest, real_digest, SM3_DIGEST_SIZE) == 0) {
            printf("  ✓ 结果正确\n");
        } else {
            printf("  ✗ 结果错误\n");
        }
    } else {
        printf("  ✗ 单字节扩展攻击失败\n");
    }
    printf("\n");
}

// 性能测试
static void performance_test() {
    printf("=== 长度扩展攻击性能测试 ===\n\n");
    
    const size_t test_sizes[] = {64, 128, 256, 512, 1024};
    const int iterations = 1000;
    
    for (int i = 0; i < 5; i++) {
        size_t data_size = test_sizes[i];
        
        // 生成测试数据
        uint8_t *data = (uint8_t*)safe_malloc(data_size);
        if (!data) continue;
        
        random_bytes(data, data_size);
        
        // 计算基础哈希
        uint8_t base_digest[SM3_DIGEST_SIZE];
        sm3_hash(data, data_size, base_digest);
        
        // 生成扩展数据
        uint8_t *extension = (uint8_t*)safe_malloc(data_size);
        if (!extension) {
            safe_free(data);
            continue;
        }
        random_bytes(extension, data_size);
        
        // 测量攻击性能
        timer_t timer;
        timer_start(&timer);
        
        for (int j = 0; j < iterations; j++) {
            uint8_t attack_digest[SM3_DIGEST_SIZE];
            sm3_length_extension_attack(
                base_digest, data_size, extension, data_size, attack_digest
            );
        }
        
        timer_stop(&timer);
        double elapsed = timer_get_elapsed_ms(&timer);
        double throughput = (iterations / (elapsed / 1000.0));
        
        printf("数据大小: %zu 字节\n", data_size);
        printf("  攻击次数: %d\n", iterations);
        printf("  总时间: %.3f ms\n", elapsed);
        printf("  平均时间: %.3f ms\n", elapsed / iterations);
        printf("  吞吐量: %.2f 攻击/秒\n\n", throughput);
        
        safe_free(data);
        safe_free(extension);
    }
}

// 安全建议
static void security_recommendations() {
    printf("=== 安全建议 ===\n\n");
    
    printf("长度扩展攻击的防护措施:\n");
    printf("1. 使用HMAC构造: HMAC(K, M) = H(K ⊕ opad || H(K ⊕ ipad || M))\n");
    printf("2. 使用密钥前缀: H(K || M)\n");
    printf("3. 使用密钥后缀: H(M || K)\n");
    printf("4. 使用双哈希: H(H(K || M) || K)\n");
    printf("5. 使用随机盐值: H(salt || M)\n\n");
    
    printf("当前SM3实现的问题:\n");
    printf("- 直接使用SM3进行消息认证码(MAC)构造容易受到长度扩展攻击\n");
    printf("- 攻击者可以在不知道密钥的情况下构造有效的MAC\n");
    printf("- 这违反了MAC的安全性要求\n\n");
    
    printf("建议的改进方案:\n");
    printf("- 实现HMAC-SM3\n");
    printf("- 在应用层添加长度验证\n");
    printf("- 使用随机盐值\n");
    printf("- 考虑使用其他抗长度扩展攻击的构造\n");
}

// 主函数
int main() {
    printf("SM3长度扩展攻击测试程序\n");
    printf("========================\n\n");
    
    // 初始化随机数生成器
    init_random();
    
    // 运行各种测试
    demonstrate_length_extension_attack();
    test_different_lengths();
    test_edge_cases();
    performance_test();
    security_recommendations();
    
    printf("=== 测试完成 ===\n");
    printf("长度扩展攻击测试程序已执行完毕。\n");
    printf("这些测试展示了SM3哈希函数在特定使用场景下的安全风险。\n");
    
    return 0;
}
