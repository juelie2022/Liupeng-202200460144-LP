#include "sm4.h"
#include "sm4_gcm.h"
#include "sm4_cpu_features.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 测试向量 */
static const struct {
    uint8_t key[16];
    uint8_t plaintext[16];
    uint8_t ciphertext[16];
} sm4_test_vectors[] = {
    {
        /* 测试向量1 - 来自SM4标准 */
        {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10},
        {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10},
        {0x68, 0x1E, 0xDF, 0x34, 0xD2, 0x06, 0x96, 0x5E, 0x86, 0xB3, 0xE9, 0x4F, 0x53, 0x6E, 0x42, 0x46}
    },
    {
        /* 测试向量2 */
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x59, 0x52, 0x98, 0xC7, 0xC6, 0xFD, 0x27, 0x1F, 0x04, 0x02, 0xF8, 0x04, 0xC3, 0x3D, 0x3F, 0x66}
    }
};

/* GCM测试向量 */
static const struct {
    uint8_t key[16];
    uint8_t iv[12];
    uint8_t aad[16];
    uint8_t plaintext[16];
    uint8_t ciphertext[16];
    uint8_t tag[16];
} gcm_test_vectors[] = {
    {
        /* 简化的测试向量 */
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
        {0xCA, 0xFE, 0xBA, 0xBE, 0xFA, 0xCE, 0xDB, 0xAD, 0xDE, 0xCA, 0xF8, 0x88},
        {0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF},
        {0xD9, 0x31, 0x32, 0x25, 0xF8, 0x84, 0x06, 0xE5, 0xA5, 0x59, 0x09, 0xC5, 0xAF, 0xF5, 0x26, 0x9A},
        {0x2A, 0x3D, 0xE8, 0x20, 0x5E, 0x15, 0x3A, 0x08, 0xA2, 0xAA, 0xFB, 0x62, 0xD6, 0x5D, 0xCC, 0x52},
        {0x9A, 0x2D, 0x5B, 0xF3, 0x06, 0xA1, 0xE8, 0x76, 0x28, 0x3F, 0xC7, 0x7B, 0x05, 0x55, 0xB9, 0x7A}
    }
};

/* 打印十六进制数据 */
static void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/* 测试基本SM4实现 */
static int test_sm4_basic(void) {
    SM4_Context ctx;
    uint8_t output[16];
    int passed = 1;
    
    printf("测试基本SM4实现...\n");
    
    for (size_t i = 0; i < sizeof(sm4_test_vectors) / sizeof(sm4_test_vectors[0]); i++) {
        printf("\n测试向量 %zu:\n", i + 1);
        
        /* 加密测试 */
        sm4_set_encrypt_key(&ctx, sm4_test_vectors[i].key);
        sm4_encrypt_block(&ctx, output, sm4_test_vectors[i].plaintext);
        
        print_hex("密钥", sm4_test_vectors[i].key, 16);
        print_hex("明文", sm4_test_vectors[i].plaintext, 16);
        print_hex("期望密文", sm4_test_vectors[i].ciphertext, 16);
        print_hex("实际密文", output, 16);
        
        if (memcmp(output, sm4_test_vectors[i].ciphertext, 16) != 0) {
            printf("加密测试失败!\n");
            passed = 0;
        } else {
            printf("加密测试通过!\n");
        }
        
        /* 解密测试 */
        sm4_set_decrypt_key(&ctx, sm4_test_vectors[i].key);
        sm4_decrypt_block(&ctx, output, sm4_test_vectors[i].ciphertext);
        
        print_hex("解密结果", output, 16);
        
        if (memcmp(output, sm4_test_vectors[i].plaintext, 16) != 0) {
            printf("解密测试失败!\n");
            passed = 0;
        } else {
            printf("解密测试通过!\n");
        }
    }
    
    return passed;
}

/* 测试T表SM4实现 */
static int test_sm4_t_table(void) {
    SM4_Context ctx;
    uint8_t output[16];
    int passed = 1;
    
    printf("\n测试T表SM4实现...\n");
    
    for (size_t i = 0; i < sizeof(sm4_test_vectors) / sizeof(sm4_test_vectors[0]); i++) {
        printf("\n测试向量 %zu:\n", i + 1);
        
        /* 加密测试 */
        sm4_set_encrypt_key(&ctx, sm4_test_vectors[i].key);
        sm4_encrypt_block(&ctx, output, sm4_test_vectors[i].plaintext);
        
        if (memcmp(output, sm4_test_vectors[i].ciphertext, 16) != 0) {
            printf("加密测试失败!\n");
            passed = 0;
        } else {
            printf("加密测试通过!\n");
        }
        
        /* 解密测试 */
        sm4_set_decrypt_key(&ctx, sm4_test_vectors[i].key);
        sm4_decrypt_block(&ctx, output, sm4_test_vectors[i].ciphertext);
        
        if (memcmp(output, sm4_test_vectors[i].plaintext, 16) != 0) {
            printf("解密测试失败!\n");
            passed = 0;
        } else {
            printf("解密测试通过!\n");
        }
    }
    
    return passed;
}

/* 测试SM4-GCM实现 */
static int test_sm4_gcm(void) {
    uint8_t ciphertext[16];
    uint8_t tag[16];
    uint8_t decrypted[16];
    int passed = 1;
    
    printf("\n测试SM4-GCM实现...\n");
    
    for (size_t i = 0; i < sizeof(gcm_test_vectors) / sizeof(gcm_test_vectors[0]); i++) {
        printf("\nGCM测试向量 %zu:\n", i + 1);
        
        /* 加密和认证 */
        sm4_gcm_encrypt_and_tag(
            gcm_test_vectors[i].key,
            gcm_test_vectors[i].iv, sizeof(gcm_test_vectors[i].iv),
            gcm_test_vectors[i].aad, sizeof(gcm_test_vectors[i].aad),
            gcm_test_vectors[i].plaintext, sizeof(gcm_test_vectors[i].plaintext),
            ciphertext, tag, sizeof(tag)
        );
        
        print_hex("密钥", gcm_test_vectors[i].key, 16);
        print_hex("IV", gcm_test_vectors[i].iv, 12);
        print_hex("AAD", gcm_test_vectors[i].aad, 16);
        print_hex("明文", gcm_test_vectors[i].plaintext, 16);
        print_hex("期望密文", gcm_test_vectors[i].ciphertext, 16);
        print_hex("实际密文", ciphertext, 16);
        print_hex("期望标签", gcm_test_vectors[i].tag, 16);
        print_hex("实际标签", tag, 16);
        
        /* 验证密文和标签 */
        if (memcmp(ciphertext, gcm_test_vectors[i].ciphertext, 16) != 0) {
            printf("GCM加密测试失败!\n");
            passed = 0;
        } else {
            printf("GCM加密测试通过!\n");
        }
        
        /* 解密和验证 */
        int result = sm4_gcm_decrypt_and_verify(
            gcm_test_vectors[i].key,
            gcm_test_vectors[i].iv, sizeof(gcm_test_vectors[i].iv),
            gcm_test_vectors[i].aad, sizeof(gcm_test_vectors[i].aad),
            gcm_test_vectors[i].ciphertext, sizeof(gcm_test_vectors[i].ciphertext),
            gcm_test_vectors[i].tag, sizeof(gcm_test_vectors[i].tag),
            decrypted
        );
        
        print_hex("解密结果", decrypted, 16);
        
        if (result != 0 || memcmp(decrypted, gcm_test_vectors[i].plaintext, 16) != 0) {
            printf("GCM解密和验证测试失败!\n");
            passed = 0;
        } else {
            printf("GCM解密和验证测试通过!\n");
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
    
    /* 运行测试 */
    if (!test_sm4_basic()) {
        passed = 0;
    }
    
    if (!test_sm4_t_table()) {
        passed = 0;
    }
    
    if (!test_sm4_gcm()) {
        passed = 0;
    }
    
    /* 输出总结果 */
    printf("\n测试结果: %s\n", passed ? "全部通过" : "部分失败");
    
    return passed ? 0 : 1;
}