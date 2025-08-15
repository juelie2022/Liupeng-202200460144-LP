#include "sm4_gcm.h"
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

int main(int argc, char *argv[]) {
    /* 测试数据 */
    uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    
    uint8_t iv[12] = {
        0xCA, 0xFE, 0xBA, 0xBE, 0xFA, 0xCE, 0xDB, 0xAD,
        0xDE, 0xCA, 0xF8, 0x88
    };
    
    uint8_t aad[20] = {
        0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
        0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF,
        0xAB, 0xAD, 0xDA, 0xD2
    };
    
    uint8_t plaintext[64] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
    };
    
    uint8_t ciphertext[64];
    uint8_t decrypted[64];
    uint8_t tag[16];
    uint8_t tag_verify[16];
    
    /* 打印测试数据 */
    print_hex("密钥", key, sizeof(key));
    print_hex("IV", iv, sizeof(iv));
    print_hex("AAD", aad, sizeof(aad));
    print_hex("明文", plaintext, sizeof(plaintext));
    
    /* 使用一步式API进行加密和认证 */
    printf("\n=== 使用一步式API ===\n");
    sm4_gcm_encrypt_and_tag(
        key, iv, sizeof(iv),
        aad, sizeof(aad),
        plaintext, sizeof(plaintext),
        ciphertext, tag, sizeof(tag)
    );
    
    print_hex("密文", ciphertext, sizeof(ciphertext));
    print_hex("认证标签", tag, sizeof(tag));
    
    /* 使用一步式API进行解密和验证 */
    int result = sm4_gcm_decrypt_and_verify(
        key, iv, sizeof(iv),
        aad, sizeof(aad),
        ciphertext, sizeof(ciphertext),
        tag, sizeof(tag),
        decrypted
    );
    
    if (result == 0) {
        printf("认证成功!\n");
        print_hex("解密", decrypted, sizeof(decrypted));
        
        /* 验证解密结果 */
        if (memcmp(plaintext, decrypted, sizeof(plaintext)) == 0) {
            printf("解密验证成功: 解密结果与原始明文匹配\n");
        } else {
            printf("错误: 解密结果与原始明文不匹配!\n");
        }
    } else {
        printf("认证失败!\n");
    }
    
    /* 使用分步式API */
    printf("\n=== 使用分步式API ===\n");
    
    SM4_GCM_Context encrypt_ctx;
    SM4_GCM_Context decrypt_ctx;
    
    /* 加密 */
    sm4_gcm_init(&encrypt_ctx, key, iv, sizeof(iv));
    sm4_gcm_aad(&encrypt_ctx, aad, sizeof(aad));
    sm4_gcm_encrypt(&encrypt_ctx, ciphertext, plaintext, sizeof(plaintext));
    sm4_gcm_finish(&encrypt_ctx, tag, sizeof(tag));
    
    print_hex("密文", ciphertext, sizeof(ciphertext));
    print_hex("认证标签", tag, sizeof(tag));
    
    /* 解密 */
    sm4_gcm_init(&decrypt_ctx, key, iv, sizeof(iv));
    sm4_gcm_aad(&decrypt_ctx, aad, sizeof(aad));
    sm4_gcm_decrypt(&decrypt_ctx, decrypted, ciphertext, sizeof(ciphertext));
    sm4_gcm_finish(&decrypt_ctx, tag_verify, sizeof(tag_verify));
    
    /* 验证标签 */
    if (memcmp(tag, tag_verify, sizeof(tag)) == 0) {
        printf("认证成功!\n");
        print_hex("解密", decrypted, sizeof(decrypted));
        
        /* 验证解密结果 */
        if (memcmp(plaintext, decrypted, sizeof(plaintext)) == 0) {
            printf("解密验证成功: 解密结果与原始明文匹配\n");
        } else {
            printf("错误: 解密结果与原始明文不匹配!\n");
        }
    } else {
        printf("认证失败!\n");
    }
    
    return 0;
}