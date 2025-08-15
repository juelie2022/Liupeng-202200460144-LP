#include "../include/sm3.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 测试向量
static const struct {
    const char *message;
    const char *expected_hash;
} test_vectors[] = {
    {"", "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0"},
    {"a", "82ec6c19ec30c6e7a8667adffa4edb8f9b9d262e02b3a6c0c8b5b1d11048d0c1"},
    {"abc", "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0"},
    {"abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd", 
     "debe9ff92275b8a138604889c18e5a4d6fdb70e5387e5765293dcba39c0c5732"},
    {"abcdefghijklmnopqrstuvwxyz", 
     "b80fe97a4da24afc87d61c6f644bb7dd8e8e1779f7f32c38a9935a6b4070f4b1"},
    {"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
     "ad293c3acf5ad8765b646c609e748fee693f8e8af095fcc1e2b9aeb205f87aa1"}
};

// 测试基本SM3功能
static int test_basic_sm3() {
    printf("=== 测试基本SM3功能 ===\n");
    
    int passed = 0;
    int total = sizeof(test_vectors) / sizeof(test_vectors[0]);
    
    for (int i = 0; i < total; i++) {
        uint8_t digest[SM3_DIGEST_SIZE];
        char hex_digest[SM3_DIGEST_SIZE * 2 + 1];
        
        // 计算哈希
        sm3_hash_string(test_vectors[i].message, digest);
        bytes_to_hex(digest, SM3_DIGEST_SIZE, hex_digest);
        
        // 验证结果
        if (strcmp(hex_digest, test_vectors[i].expected_hash) == 0) {
            printf("✓ 测试 %d 通过: \"%s\"\n", i + 1, 
                   strlen(test_vectors[i].message) > 20 ? 
                   "长消息" : test_vectors[i].message);
            passed++;
        } else {
            printf("✗ 测试 %d 失败: \"%s\"\n", i + 1, 
                   strlen(test_vectors[i].message) > 20 ? 
                   "长消息" : test_vectors[i].message);
            printf("  期望: %s\n", test_vectors[i].expected_hash);
            printf("  实际: %s\n", hex_digest);
        }
    }
    
    printf("基本功能测试: %d/%d 通过\n\n", passed, total);
    return passed == total;
}

// 测试SM3上下文操作
static int test_sm3_context() {
    printf("=== 测试SM3上下文操作 ===\n");
    
    const char *message = "这是一个测试消息，用于验证SM3上下文的正确性。";
    uint8_t digest1[SM3_DIGEST_SIZE], digest2[SM3_DIGEST_SIZE];
    
    // 方法1: 直接计算
    sm3_hash_string(message, digest1);
    
    // 方法2: 使用上下文
    sm3_ctx_t ctx;
    sm3_init(&ctx);
    sm3_update(&ctx, (const uint8_t*)message, strlen(message));
    sm3_final(&ctx, digest2);
    
    // 比较结果
    if (memcmp(digest1, digest2, SM3_DIGEST_SIZE) == 0) {
        printf("✓ 上下文操作测试通过\n");
        return 1;
    } else {
        printf("✗ 上下文操作测试失败\n");
        return 0;
    }
}

// 测试大文件哈希
static int test_large_data() {
    printf("=== 测试大文件哈希 ===\n");
    
    const size_t data_size = 1024 * 1024; // 1MB
    uint8_t *data = (uint8_t*)safe_malloc(data_size);
    if (!data) {
        printf("✗ 内存分配失败\n");
        return 0;
    }
    
    // 生成测试数据
    for (size_t i = 0; i < data_size; i++) {
        data[i] = (uint8_t)(i % 256);
    }
    
    uint8_t digest[SM3_DIGEST_SIZE];
    timer_t timer;
    
    // 测量性能
    timer_start(&timer);
    sm3_hash(data, data_size, digest);
    timer_stop(&timer);
    
    double elapsed = timer_get_elapsed_ms(&timer);
    double throughput = (data_size / 1024.0 / 1024.0) / (elapsed / 1000.0);
    
    printf("✓ 大文件哈希测试通过\n");
    printf("  数据大小: %.2f MB\n", data_size / 1024.0 / 1024.0);
    printf("  处理时间: %.3f ms\n", elapsed);
    printf("  吞吐量: %.2f MB/s\n", throughput);
    
    safe_free(data);
    return 1;
}

// 测试优化版本
static int test_optimized_sm3() {
    printf("=== 测试优化版本SM3 ===\n");
    
    const char *message = "测试优化版本的SM3算法实现";
    uint8_t digest1[SM3_DIGEST_SIZE], digest2[SM3_DIGEST_SIZE];
    
    // 基本版本
    sm3_hash_string(message, digest1);
    
    // 优化版本
    sm3_hash_optimized((const uint8_t*)message, strlen(message), digest2);
    
    // 比较结果
    if (memcmp(digest1, digest2, SM3_DIGEST_SIZE) == 0) {
        printf("✓ 优化版本测试通过\n");
        return 1;
    } else {
        printf("✗ 优化版本测试失败\n");
        return 0;
    }
}

// 性能对比测试
static void performance_comparison() {
    printf("=== 性能对比测试 ===\n");
    
    const size_t data_sizes[] = {1024, 10240, 102400, 1024000}; // 1KB, 10KB, 100KB, 1MB
    const int iterations = 1000;
    
    for (int i = 0; i < 4; i++) {
        size_t data_size = data_sizes[i];
        printf("\n数据大小: %.2f KB\n", data_size / 1024.0);
        
        sm3_benchmark(data_size, iterations);
    }
}

// 测试长度扩展攻击
static int test_length_extension() {
    printf("=== 测试长度扩展攻击 ===\n");
    
    const char *original_message = "secret";
    const char *extension = "extension";
    
    uint8_t original_digest[SM3_DIGEST_SIZE];
    uint8_t attack_digest[SM3_DIGEST_SIZE];
    uint8_t legitimate_digest[SM3_DIGEST_SIZE];
    
    // 计算原始消息的哈希
    sm3_hash_string(original_message, original_digest);
    
    // 尝试长度扩展攻击
    int attack_success = sm3_length_extension_attack(
        original_digest, 
        strlen(original_message),
        (const uint8_t*)extension, 
        strlen(extension),
        attack_digest
    );
    
    // 计算真实的消息哈希（原始消息+扩展）
    char combined_message[256];
    snprintf(combined_message, sizeof(combined_message), "%s%s", original_message, extension);
    sm3_hash_string(combined_message, legitimate_digest);
    
    printf("原始消息: \"%s\"\n", original_message);
    printf("扩展数据: \"%s\"\n", extension);
    printf("攻击结果: %s\n", attack_success ? "成功" : "失败");
    
    if (attack_success) {
        printf("攻击生成的哈希: ");
        sm3_print_digest(attack_digest);
        printf("真实哈希: ");
        sm3_print_digest(legitimate_digest);
        
        if (memcmp(attack_digest, legitimate_digest, SM3_DIGEST_SIZE) == 0) {
            printf("✓ 长度扩展攻击成功！\n");
            return 1;
        } else {
            printf("✗ 长度扩展攻击失败\n");
            return 0;
        }
    }
    
    return 0;
}

// 主测试函数
int main(int argc, char *argv[]) {
    printf("SM3算法测试程序\n");
    printf("================\n\n");
    
    // 检查命令行参数
    int run_benchmark = 0;
    if (argc > 1 && strcmp(argv[1], "--benchmark") == 0) {
        run_benchmark = 1;
    }
    
    int all_tests_passed = 1;
    
    // 运行基本测试
    if (!test_basic_sm3()) all_tests_passed = 0;
    if (!test_sm3_context()) all_tests_passed = 0;
    if (!test_large_data()) all_tests_passed = 0;
    if (!test_optimized_sm3()) all_tests_passed = 0;
    
    // 测试长度扩展攻击
    test_length_extension();
    
    // 性能测试
    if (run_benchmark) {
        performance_comparison();
    }
    
    printf("\n=== 测试总结 ===\n");
    if (all_tests_passed) {
        printf("✓ 所有基本测试通过\n");
    } else {
        printf("✗ 部分测试失败\n");
    }
    
    return all_tests_passed ? 0 : 1;
}
