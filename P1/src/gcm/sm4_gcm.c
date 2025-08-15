#include "sm4_gcm.h"
#include <string.h>

/* GF(2^128)上的乘法 */
static void gf128_mul(uint8_t *r, const uint8_t *x, const uint8_t *y) {
    uint8_t z[16] = {0};
    uint8_t v[16];
    uint8_t mask;
    int i, j;
    
    memcpy(v, y, 16);
    
    for (i = 0; i < 16; i++) {
        for (j = 7; j >= 0; j--) {
            mask = (x[i] >> j) & 1;
            
            if (mask) {
                for (int k = 0; k < 16; k++) {
                    z[k] ^= v[k];
                }
            }
            
            /* 右移一位，如果最低位为1，则异或上多项式 */
            mask = v[15] & 1;
            for (int k = 15; k > 0; k--) {
                v[k] = (v[k] >> 1) | ((v[k-1] & 1) << 7);
            }
            v[0] >>= 1;
            
            /* 如果最低位为1，异或上多项式 x^128 + x^7 + x^2 + x + 1 */
            if (mask) {
                v[0] ^= 0xe1; /* 0b11100001 */
            }
        }
    }
    
    memcpy(r, z, 16);
}

/* GHASH函数 */
static void ghash(uint8_t *output, const uint8_t *h, const uint8_t *input, size_t len) {
    uint8_t tmp[16];
    size_t i;
    
    memcpy(tmp, output, 16);
    
    for (i = 0; i < len; i += 16) {
        /* 异或输入 */
        for (int j = 0; j < 16; j++) {
            tmp[j] ^= input[i + j];
        }
        
        /* GF(2^128)上的乘法 */
        gf128_mul(tmp, tmp, h);
    }
    
    memcpy(output, tmp, 16);
}

/* 增加计数器 */
static void increment_counter(uint8_t *counter) {
    int i;
    
    for (i = 15; i >= 12; i--) {
        if (++counter[i] != 0) {
            break;
        }
    }
}

/* 初始化SM4-GCM上下文 */
int sm4_gcm_init(SM4_GCM_Context *ctx, const uint8_t *key, const uint8_t *iv, size_t iv_len) {
    uint8_t zero[SM4_BLOCK_SIZE] = {0};
    
    /* 初始化SM4上下文 */
    sm4_set_encrypt_key(&ctx->cipher_ctx, key);
    
    /* 计算GHASH密钥H = E_K(0) */
    sm4_encrypt_block(&ctx->cipher_ctx, ctx->H, zero);
    
    /* 初始化计数器 */
    if (iv_len == 12) {
        /* 96位IV */
        memcpy(ctx->J0, iv, 12);
        ctx->J0[12] = 0;
        ctx->J0[13] = 0;
        ctx->J0[14] = 0;
        ctx->J0[15] = 1;
    } else {
        /* 其他长度IV */
        uint8_t tmp[16] = {0};
        size_t full_blocks = iv_len / 16;
        size_t remainder = iv_len % 16;
        
        /* 处理完整块 */
        for (size_t i = 0; i < full_blocks; i++) {
            ghash(tmp, ctx->H, iv + i * 16, 16);
        }
        
        /* 处理剩余字节 */
        if (remainder) {
            uint8_t last_block[16] = {0};
            memcpy(last_block, iv + full_blocks * 16, remainder);
            ghash(tmp, ctx->H, last_block, 16);
        }
        
        /* 添加长度信息 */
        uint8_t len_block[16] = {0};
        uint64_t bit_len = iv_len * 8;
        len_block[8] = (uint8_t)(bit_len >> 56);
        len_block[9] = (uint8_t)(bit_len >> 48);
        len_block[10] = (uint8_t)(bit_len >> 40);
        len_block[11] = (uint8_t)(bit_len >> 32);
        len_block[12] = (uint8_t)(bit_len >> 24);
        len_block[13] = (uint8_t)(bit_len >> 16);
        len_block[14] = (uint8_t)(bit_len >> 8);
        len_block[15] = (uint8_t)bit_len;
        
        ghash(tmp, ctx->H, len_block, 16);
        memcpy(ctx->J0, tmp, 16);
    }
    
    /* 初始化其他字段 */
    ctx->len_a = 0;
    ctx->len_c = 0;
    ctx->buf_len = 0;
    memset(ctx->final_ghash, 0, SM4_BLOCK_SIZE);
    
    return 0;
}

/* 处理附加认证数据 */
int sm4_gcm_aad(SM4_GCM_Context *ctx, const uint8_t *aad, size_t aad_len) {
    size_t full_blocks = aad_len / 16;
    size_t remainder = aad_len % 16;
    
    /* 处理完整块 */
    if (full_blocks > 0) {
        ghash(ctx->final_ghash, ctx->H, aad, full_blocks * 16);
    }
    
    /* 处理剩余字节 */
    if (remainder) {
        uint8_t last_block[16] = {0};
        memcpy(last_block, aad + full_blocks * 16, remainder);
        ghash(ctx->final_ghash, ctx->H, last_block, 16);
    }
    
    ctx->len_a += aad_len;
    
    return 0;
}

/* SM4-GCM加密 */
int sm4_gcm_encrypt(SM4_GCM_Context *ctx, uint8_t *out, const uint8_t *in, size_t len) {
    uint8_t counter[SM4_BLOCK_SIZE];
    uint8_t encrypted_counter[SM4_BLOCK_SIZE];
    size_t i, j;
    
    /* 复制初始计数器 */
    memcpy(counter, ctx->J0, SM4_BLOCK_SIZE);
    increment_counter(counter); /* 从1开始 */
    
    /* 处理完整块 */
    for (i = 0; i + SM4_BLOCK_SIZE <= len; i += SM4_BLOCK_SIZE) {
        /* 加密计数器 */
        sm4_encrypt_block(&ctx->cipher_ctx, encrypted_counter, counter);
        
        /* 异或明文得到密文 */
        for (j = 0; j < SM4_BLOCK_SIZE; j++) {
            out[i + j] = in[i + j] ^ encrypted_counter[j];
        }
        
        /* 更新GHASH */
        ghash(ctx->final_ghash, ctx->H, out + i, SM4_BLOCK_SIZE);
        
        /* 增加计数器 */
        increment_counter(counter);
    }
    
    /* 处理最后一个不完整块 */
    if (i < len) {
        size_t remainder = len - i;
        
        /* 加密计数器 */
        sm4_encrypt_block(&ctx->cipher_ctx, encrypted_counter, counter);
        
        /* 异或明文得到密文 */
        for (j = 0; j < remainder; j++) {
            out[i + j] = in[i + j] ^ encrypted_counter[j];
        }
        
        /* 更新GHASH */
        uint8_t last_block[SM4_BLOCK_SIZE] = {0};
        memcpy(last_block, out + i, remainder);
        ghash(ctx->final_ghash, ctx->H, last_block, SM4_BLOCK_SIZE);
    }
    
    ctx->len_c += len;
    
    return 0;
}

/* SM4-GCM解密 */
int sm4_gcm_decrypt(SM4_GCM_Context *ctx, uint8_t *out, const uint8_t *in, size_t len) {
    uint8_t counter[SM4_BLOCK_SIZE];
    uint8_t encrypted_counter[SM4_BLOCK_SIZE];
    size_t i, j;
    
    /* 复制初始计数器 */
    memcpy(counter, ctx->J0, SM4_BLOCK_SIZE);
    increment_counter(counter); /* 从1开始 */
    
    /* 处理完整块 */
    for (i = 0; i + SM4_BLOCK_SIZE <= len; i += SM4_BLOCK_SIZE) {
        /* 更新GHASH */
        ghash(ctx->final_ghash, ctx->H, in + i, SM4_BLOCK_SIZE);
        
        /* 加密计数器 */
        sm4_encrypt_block(&ctx->cipher_ctx, encrypted_counter, counter);
        
        /* 异或密文得到明文 */
        for (j = 0; j < SM4_BLOCK_SIZE; j++) {
            out[i + j] = in[i + j] ^ encrypted_counter[j];
        }
        
        /* 增加计数器 */
        increment_counter(counter);
    }
    
    /* 处理最后一个不完整块 */
    if (i < len) {
        size_t remainder = len - i;
        
        /* 更新GHASH */
        uint8_t last_block[SM4_BLOCK_SIZE] = {0};
        memcpy(last_block, in + i, remainder);
        ghash(ctx->final_ghash, ctx->H, last_block, SM4_BLOCK_SIZE);
        
        /* 加密计数器 */
        sm4_encrypt_block(&ctx->cipher_ctx, encrypted_counter, counter);
        
        /* 异或密文得到明文 */
        for (j = 0; j < remainder; j++) {
            out[i + j] = in[i + j] ^ encrypted_counter[j];
        }
    }
    
    ctx->len_c += len;
    
    return 0;
}

/* 完成SM4-GCM操作并生成认证标签 */
int sm4_gcm_finish(SM4_GCM_Context *ctx, uint8_t *tag, size_t tag_len) {
    uint8_t len_block[SM4_BLOCK_SIZE];
    uint8_t auth_tag[SM4_BLOCK_SIZE];
    
    /* 添加长度信息 */
    uint64_t bit_len_a = ctx->len_a * 8;
    uint64_t bit_len_c = ctx->len_c * 8;
    
    memset(len_block, 0, SM4_BLOCK_SIZE);
    len_block[0] = (uint8_t)(bit_len_a >> 56);
    len_block[1] = (uint8_t)(bit_len_a >> 48);
    len_block[2] = (uint8_t)(bit_len_a >> 40);
    len_block[3] = (uint8_t)(bit_len_a >> 32);
    len_block[4] = (uint8_t)(bit_len_a >> 24);
    len_block[5] = (uint8_t)(bit_len_a >> 16);
    len_block[6] = (uint8_t)(bit_len_a >> 8);
    len_block[7] = (uint8_t)bit_len_a;
    len_block[8] = (uint8_t)(bit_len_c >> 56);
    len_block[9] = (uint8_t)(bit_len_c >> 48);
    len_block[10] = (uint8_t)(bit_len_c >> 40);
    len_block[11] = (uint8_t)(bit_len_c >> 32);
    len_block[12] = (uint8_t)(bit_len_c >> 24);
    len_block[13] = (uint8_t)(bit_len_c >> 16);
    len_block[14] = (uint8_t)(bit_len_c >> 8);
    len_block[15] = (uint8_t)bit_len_c;
    
    ghash(ctx->final_ghash, ctx->H, len_block, SM4_BLOCK_SIZE);
    
    /* 加密初始计数器 */
    sm4_encrypt_block(&ctx->cipher_ctx, auth_tag, ctx->J0);
    
    /* 异或GHASH结果得到认证标签 */
    for (size_t i = 0; i < SM4_BLOCK_SIZE; i++) {
        auth_tag[i] ^= ctx->final_ghash[i];
    }
    
    /* 复制认证标签 */
    memcpy(tag, auth_tag, tag_len < SM4_BLOCK_SIZE ? tag_len : SM4_BLOCK_SIZE);
    
    return 0;
}

/* 一步完成SM4-GCM加密和认证 */
int sm4_gcm_encrypt_and_tag(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                           const uint8_t *aad, size_t aad_len,
                           const uint8_t *in, size_t in_len,
                           uint8_t *out, uint8_t *tag, size_t tag_len) {
    SM4_GCM_Context ctx;
    
    /* 初始化 */
    sm4_gcm_init(&ctx, key, iv, iv_len);
    
    /* 处理AAD */
    if (aad_len > 0) {
        sm4_gcm_aad(&ctx, aad, aad_len);
    }
    
    /* 加密 */
    sm4_gcm_encrypt(&ctx, out, in, in_len);
    
    /* 生成标签 */
    sm4_gcm_finish(&ctx, tag, tag_len);
    
    return 0;
}

/* 一步完成SM4-GCM解密和验证 */
int sm4_gcm_decrypt_and_verify(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                              const uint8_t *aad, size_t aad_len,
                              const uint8_t *in, size_t in_len,
                              const uint8_t *tag, size_t tag_len,
                              uint8_t *out) {
    SM4_GCM_Context ctx;
    uint8_t calculated_tag[SM4_BLOCK_SIZE];
    int result = 0;
    
    /* 初始化 */
    sm4_gcm_init(&ctx, key, iv, iv_len);
    
    /* 处理AAD */
    if (aad_len > 0) {
        sm4_gcm_aad(&ctx, aad, aad_len);
    }
    
    /* 解密 */
    sm4_gcm_decrypt(&ctx, out, in, in_len);
    
    /* 生成标签 */
    sm4_gcm_finish(&ctx, calculated_tag, SM4_BLOCK_SIZE);
    
    /* 验证标签 */
    for (size_t i = 0; i < tag_len; i++) {
        if (calculated_tag[i] != tag[i]) {
            result = -1;
            break;
        }
    }
    
    /* 如果验证失败，清除输出 */
    if (result != 0) {
        memset(out, 0, in_len);
    }
    
    return result;
}