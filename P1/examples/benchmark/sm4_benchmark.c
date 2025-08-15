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

/* GCM测试数据 */
static uint8_t iv[12] = {
    0xCA, 0xFE, 0xBA, 0xBE, 0xFA, 0xCE, 0xDB, 0xAD,
    0xDE, 0xCA, 0xF8, 0x88
};

static uint8_t aad[16] = {
    0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
    0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF
};

static uint8_t gcm_plaintext[64];
static uint8_t gcm_ciphertext[64];
static uint8_t gcm_tag[16];

/* 全局上下文 */
static SM4_Context basic_encrypt_ctx;
static SM4_Context basic_decrypt_ctx;
static SM4_Context t_table_encrypt_ctx;
static SM4_Context t_table_decrypt_ctx;
static SM4_Context aesni_encrypt_ctx;
static SM4_Context aesni_decrypt_ctx;
static SM4_Context modern_encrypt_ctx;
static SM4_Context modern_decrypt_ctx;
static SM4_GCM_Context gcm_ctx;

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

/* 测试函数 - GCM模式 */
static void test_gcm_encrypt(void) {
    sm4_gcm_encrypt_and_tag(
        key, iv, sizeof(iv),
        aad, sizeof(aad),
        gcm_plaintext, sizeof(gcm_plaintext),
        gcm_ciphertext, gcm_tag, sizeof(gcm_tag)
    );
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    int gcm_iterations = 10000;
    double time_used;
    SM4_CPU_Features features;
    
    /* 初始化测试数据 */
    for (int i = 0; i < sizeof(gcm_plaintext); i++) {
        gcm_plaintext[i] = (uint8_t)i;
    }
    
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
    
    /* GCM模式 */
    printf("\nGCM模式 (%d 次迭代):\n", gcm_iterations);
    
    time_used = measure_time(test_gcm_encrypt, gcm_iterations);
    printf("  加密+认证: %.6f 秒 (%.2f MB/s)\n", 
           time_used, 
           (gcm_iterations * sizeof(gcm_plaintext)) / (time_used * 1024.0 * 1024.0));
    
    return 0;
}