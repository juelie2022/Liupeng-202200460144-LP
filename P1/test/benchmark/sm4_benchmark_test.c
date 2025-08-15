#include "sm4.h"
#include "sm4_gcm.h"
#include "sm4_cpu_features.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 测量执行时间 */
static double measure_time(void (*func)(void), int iterations) {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    for (int i = 0; i < iterations; i++) {
        func();
    }
    end = clock();
    
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    return cpu_time_used;
}

/* 测试数据 */
static uint8_t key[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
};

static uint8_t plaintext[16] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
};

static uint8_t ciphertext[16];
static uint8_t decrypted[16];

/* 全局上下文 */
static SM4_Context basic_encrypt_ctx;
static SM4_Context basic_decrypt_ctx;
static SM4_Context t_table_encrypt_ctx;
static SM4_Context t_table_decrypt_ctx;
static SM4_Context aesni_encrypt_ctx;
static SM4_Context aesni_decrypt_ctx;
static SM4_Context modern_encrypt_ctx;
static SM4_Context modern_decrypt_ctx;

/* 测试函数 - 基本实现 */
static void test_basic_encrypt(void) {
    sm4_encrypt_block(&basic_encrypt_ctx, ciphertext, plaintext);
}

static void test_basic_decrypt(void) {
    sm4_decrypt_block(&basic_decrypt_ctx, decrypted, ciphertext);
}

/* 测试函数 - T表实现 */
static void test_t_table_encrypt(void) {
    sm4_encrypt_block(&t_table_encrypt_ctx, ciphertext, plaintext);
}

static void test_t_table_decrypt(void) {
    sm4_decrypt_block(&t_table_decrypt_ctx, decrypted, ciphertext);
}

/* 测试函数 - AESNI实现 */
static void test_aesni_encrypt(void) {
    sm4_encrypt_block(&aesni_encrypt_ctx, ciphertext, plaintext);
}

static void test_aesni_decrypt(void) {
    sm4_decrypt_block(&aesni_decrypt_ctx, decrypted, ciphertext);
}

/* 测试函数 - 现代指令集实现 */
static void test_modern_encrypt(void) {
    sm4_encrypt_block(&modern_encrypt_ctx, ciphertext, plaintext);
}

static void test_modern_decrypt(void) {
    sm4_decrypt_block(&modern_decrypt_ctx, decrypted, ciphertext);
}

/* 性能测试 */
static int benchmark_implementations(void) {
    int iterations = 1000000;
    double time_used;
    SM4_CPU_Features features;
    
    /* 检测CPU特性 */
    features = sm4_get_cpu_features();
    
    /* 初始化上下文 */
    sm4_set_encrypt_key(&basic_encrypt_ctx, key);
    sm4_set_decrypt_key(&basic_decrypt_ctx, key);
    
    sm4_set_encrypt_key(&t_table_encrypt_ctx, key);
    sm4_set_decrypt_key(&t_table_decrypt_ctx, key);
    
    sm4_set_encrypt_key(&aesni_encrypt_ctx, key);
    sm4_set_decrypt_key(&aesni_decrypt_ctx, key);
    
    sm4_set_encrypt_key(&modern_encrypt_ctx, key);
    sm4_set_decrypt_key(&modern_decrypt_ctx, key);
    
    /* 性能测试 */
    printf("执行性能测试...\n\n");
    
    /* 基本实现 */
    printf("基本实现 (%d 次迭代):\n", iterations);
    
    time_used = measure_time(test_basic_encrypt, iterations);
    printf("  加密: %.6f 秒 (%.2f MB/s)\n", 
           time_used, 
           (iterations * 16.0) / (time_used * 1024.0 * 1024.0));
    
    time_used = measure_time(test_basic_decrypt, iterations);
    printf("  解密: %.6f 秒 (%.2f MB/s)\n", 
           time_used, 
           (iterations * 16.0) / (time_used * 1024.0 * 1024.0));
    
    /* T表实现 */
    printf("\nT表实现 (%d 次迭代):\n", iterations);
    
    time_used = measure_time(test_t_table_encrypt, iterations);
    printf("  加密: %.6f 秒 (%.2f MB/s)\n", 
           time_used, 
           (iterations * 16.0) / (time_used * 1024.0 * 1024.0));
    
    time_used = measure_time(test_t_table_decrypt, iterations);
    printf("  解密: %.6f 秒 (%.2f MB/s)\n", 
           time_used, 
           (iterations * 16.0) / (time_used * 1024.0 * 1024.0));
    
    /* AESNI实现 */
    if (features.has_aesni) {
        printf("\nAESNI实现 (%d 次迭代):\n", iterations);
        
        time_used = measure_time(test_aesni_encrypt, iterations);
        printf("  加密: %.6f 秒 (%.2f MB/s)\n", 
               time_used, 
               (iterations * 16.0) / (time_used * 1024.0 * 1024.0));
        
        time_used = measure_time(test_aesni_decrypt, iterations);
        printf("  解密: %.6f 秒 (%.2f MB/s)\n", 
               time_used, 
               (iterations * 16.0) / (time_used * 1024.0 * 1024.0));
    }
    
    /* 现代指令集实现 */
    if (features.has_gfni) {
        printf("\n现代指令集实现 (%d 次迭代):\n", iterations);
        
        time_used = measure_time(test_modern_encrypt, iterations);
        printf("  加密: %.6f 秒 (%.2f MB/s)\n", 
               time_used, 
               (iterations * 16.0) / (time_used * 1024.0 * 1024.0));
        
        time_used = measure_time(test_modern_decrypt, iterations);
        printf("  解密: %.6f 秒 (%.2f MB/s)\n", 
               time_used, 
               (iterations * 16.0) / (time_used * 1024.0 * 1024.0));
    }
    
    return 0;
}

/* 验证不同实现的一致性 */
static int verify_implementations(void) {
    uint8_t basic_output[16];
    uint8_t t_table_output[16];
    uint8_t aesni_output[16];
    uint8_t modern_output[16];
    SM4_CPU_Features features;
    int passed = 1;
    
    /* 检测CPU特性 */
    features = sm4_get_cpu_features();
    
    printf("验证不同实现的一致性...\n\n");
    
    /* 初始化上下文 */
    sm4_set_encrypt_key(&basic_encrypt_ctx, key);
    sm4_set_encrypt_key(&t_table_encrypt_ctx, key);
    sm4_set_encrypt_key(&aesni_encrypt_ctx, key);
    sm4_set_encrypt_key(&modern_encrypt_ctx, key);
    
    /* 加密测试 */
    sm4_encrypt_block(&basic_encrypt_ctx, basic_output, plaintext);
    sm4_encrypt_block(&t_table_encrypt_ctx, t_table_output, plaintext);
    
    printf("基本实现与T表实现比较: ");
    if (memcmp(basic_output, t_table_output, 16) != 0) {
        printf("不一致!\n");
        passed = 0;
    } else {
        printf("一致\n");
    }
    
    if (features.has_aesni) {
        sm4_encrypt_block(&aesni_encrypt_ctx, aesni_output, plaintext);
        
        printf("基本实现与AESNI实现比较: ");
        if (memcmp(basic_output, aesni_output, 16) != 0) {
            printf("不一致!\n");
            passed = 0;
        } else {
            printf("一致\n");
        }
    }
    
    if (features.has_gfni) {
        sm4_encrypt_block(&modern_encrypt_ctx, modern_output, plaintext);
        
        printf("基本实现与现代指令集实现比较: ");
        if (memcmp(basic_output, modern_output, 16) != 0) {
            printf("不一致!\n");
            passed = 0;
        } else {
            printf("一致\n");
        }
    }
    
    /* 初始化解密上下文 */
    sm4_set_decrypt_key(&basic_decrypt_ctx, key);
    sm4_set_decrypt_key(&t_table_decrypt_ctx, key);
    sm4_set_decrypt_key(&aesni_decrypt_ctx, key);
    sm4_set_decrypt_key(&modern_decrypt_ctx, key);
    
    /* 解密测试 */
    sm4_decrypt_block(&basic_decrypt_ctx, basic_output, basic_output);
    sm4_decrypt_block(&t_table_decrypt_ctx, t_table_output, t_table_output);
    
    printf("\n解密后与原始明文比较:\n");
    
    printf("基本实现: ");
    if (memcmp(basic_output, plaintext, 16) != 0) {
        printf("不一致!\n");
        passed = 0;
    } else {
        printf("一致\n");
    }
    
    printf("T表实现: ");
    if (memcmp(t_table_output, plaintext, 16) != 0) {
        printf("不一致!\n");
        passed = 0;
    } else {
        printf("一致\n");
    }
    
    if (features.has_aesni) {
        sm4_decrypt_block(&aesni_decrypt_ctx, aesni_output, aesni_output);
        
        printf("AESNI实现: ");
        if (memcmp(aesni_output, plaintext, 16) != 0) {
            printf("不一致!\n");
            passed = 0;
        } else {
            printf("一致\n");
        }
    }
    
    if (features.has_gfni) {
        sm4_decrypt_block(&modern_decrypt_ctx, modern_output, modern_output);
        
        printf("现代指令集实现: ");
        if (memcmp(modern_output, plaintext, 16) != 0) {
            printf("不一致!\n");
            passed = 0;
        } else {
            printf("一致\n");
        }
    }
    
    return passed;
}

int main(int argc, char *argv[]) {
    int passed = 1;
    SM4_CPU_Features features;
    
    /* 检测CPU特性 */
    features = sm4_get_cpu_features();
    printf("CPU特性检测:\n");
    printf("  SSE2: %s\n", features.has_sse2 ? "支持" : "不支持");
    printf("  AES-NI: %s\n", features.has_aesni ? "支持" : "不支持");
    printf("  AVX: %s\n", features.has_avx ? "支持" : "不支持");
    printf("  AVX2: %s\n", features.has_avx2 ? "支持" : "不支持");
    printf("  AVX-512F: %s\n", features.has_avx512f ? "支持" : "不支持");
    printf("  GFNI: %s\n", features.has_gfni ? "支持" : "不支持");
    printf("  VAES: %s\n", features.has_vaes ? "支持" : "不支持");
    printf("  VPCLMULQDQ: %s\n", features.has_vpclmulqdq ? "支持" : "不支持");
    
    printf("\n最佳SM4实现: %s\n\n", sm4_get_best_implementation());
    
    /* 验证不同实现的一致性 */
    if (!verify_implementations()) {
        passed = 0;
    }
    
    /* 性能测试 */
    benchmark_implementations();
    
    /* 输出总结果 */
    printf("\n测试结果: %s\n", passed ? "全部通过" : "部分失败");
    
    return passed ? 0 : 1;
}