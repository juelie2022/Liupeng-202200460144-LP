#include "sm4.h"
#include "sm4_cpu_features.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 打印十六进制数据 */
static void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

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
static SM4_Context encrypt_ctx;
static SM4_Context decrypt_ctx;

/* 测试函数 */
static void test_encrypt(void) {
    sm4_encrypt_block(&encrypt_ctx, ciphertext, plaintext);
}

static void test_decrypt(void) {
    sm4_decrypt_block(&decrypt_ctx, decrypted, ciphertext);
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    double encrypt_time, decrypt_time;
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
    
    /* 初始化上下文 */
    sm4_set_encrypt_key(&encrypt_ctx, key);
    sm4_set_decrypt_key(&decrypt_ctx, key);
    
    /* 单次加解密测试 */
    sm4_encrypt_block(&encrypt_ctx, ciphertext, plaintext);
    sm4_decrypt_block(&decrypt_ctx, decrypted, ciphertext);
    
    print_hex("密钥", key, 16);
    print_hex("明文", plaintext, 16);
    print_hex("密文", ciphertext, 16);
    print_hex("解密", decrypted, 16);
    
    /* 验证解密结果 */
    if (memcmp(plaintext, decrypted, 16) != 0) {
        printf("\n错误: 解密结果与原始明文不匹配!\n");
        return 1;
    }
    
    printf("\n解密验证成功: 解密结果与原始明文匹配\n");
    
    /* 性能测试 */
    printf("\n执行性能测试 (%d 次迭代)...\n", iterations);
    
    encrypt_time = measure_time(test_encrypt, iterations);
    decrypt_time = measure_time(test_decrypt, iterations);
    
    printf("加密: %.6f 秒 (%.2f MB/s)\n", 
           encrypt_time, 
           (iterations * 16.0) / (encrypt_time * 1024.0 * 1024.0));
    
    printf("解密: %.6f 秒 (%.2f MB/s)\n", 
           decrypt_time, 
           (iterations * 16.0) / (decrypt_time * 1024.0 * 1024.0));
    
    return 0;
}