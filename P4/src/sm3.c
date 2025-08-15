#include "sm3.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>

// SM3初始向量 (IV)
const uint32_t SM3_IV[SM3_STATE_SIZE] = {
    0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
};

// SM3常量T
static const uint32_t SM3_T[64] = {
    0x79CC4519, 0x79CC4519, 0x79CC4519, 0x79CC4519,
    0x79CC4519, 0x79CC4519, 0x79CC4519, 0x79CC4519,
    0x79CC4519, 0x79CC4519, 0x79CC4519, 0x79CC4519,
    0x79CC4519, 0x79CC4519, 0x79CC4519, 0x79CC4519,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A,
    0x7A879D8A, 0x7A879D8A, 0x7A879D8A, 0x7A879D8A
};

// 左循环移位宏
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// SM3基本函数
static uint32_t FF(uint32_t x, uint32_t y, uint32_t z, int j) {
    if (j < 16) {
        return x ^ y ^ z;
    } else {
        return (x & y) | (x & z) | (y & z);
    }
}

static uint32_t GG(uint32_t x, uint32_t y, uint32_t z, int j) {
    if (j < 16) {
        return x ^ y ^ z;
    } else {
        return (x & y) | (~x & z);
    }
}

static uint32_t P0(uint32_t x) {
    return x ^ ROTL(x, 9) ^ ROTL(x, 17);
}

static uint32_t P1(uint32_t x) {
    return x ^ ROTL(x, 15) ^ ROTL(x, 23);
}

// SM3压缩函数
static void sm3_compress(uint32_t state[SM3_STATE_SIZE], const uint8_t block[SM3_BLOCK_SIZE]) {
    uint32_t W[68];
    uint32_t W1[64];
    uint32_t A, B, C, D, E, F, G, H;
    uint32_t SS1, SS2, TT1, TT2;
    int j;
    
    // 消息扩展
    for (j = 0; j < 16; j++) {
        W[j] = ((uint32_t)block[j * 4] << 24) |
                ((uint32_t)block[j * 4 + 1] << 16) |
                ((uint32_t)block[j * 4 + 2] << 8) |
                ((uint32_t)block[j * 4 + 3]);
    }
    
    for (j = 16; j < 68; j++) {
        W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROTL(W[j - 3], 15)) ^ 
                ROTL(W[j - 13], 7) ^ W[j - 6];
    }
    
    for (j = 0; j < 64; j++) {
        W1[j] = W[j] ^ W[j + 4];
    }
    
    // 初始化工作变量
    A = state[0];
    B = state[1];
    C = state[2];
    D = state[3];
    E = state[4];
    F = state[5];
    G = state[6];
    H = state[7];
    
    // 主循环
    for (j = 0; j < 64; j++) {
        SS1 = ROTL((ROTL(A, 12) + E + ROTL(SM3_T[j], j)), 7);
        SS2 = SS1 ^ ROTL(A, 12);
        TT1 = FF(A, B, C, j) + D + SS2 + W1[j];
        TT2 = GG(E, F, G, j) + H + SS1 + W[j];
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = P0(TT2);
    }
    
    // 更新状态
    state[0] ^= A;
    state[1] ^= B;
    state[2] ^= C;
    state[3] ^= D;
    state[4] ^= E;
    state[5] ^= F;
    state[6] ^= G;
    state[7] ^= H;
}

// 初始化SM3上下文
void sm3_init(sm3_ctx_t *ctx) {
    if (!ctx) return;
    
    memcpy(ctx->state, SM3_IV, sizeof(SM3_IV));
    ctx->count = 0;
    ctx->buffer_len = 0;
    memset(ctx->buffer, 0, SM3_BLOCK_SIZE);
}

// 更新SM3上下文
void sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t len) {
    if (!ctx || !data) return;
    
    ctx->count += len;
    
    // 处理缓冲区中已有的数据
    if (ctx->buffer_len > 0) {
        size_t copy_len = SM3_BLOCK_SIZE - ctx->buffer_len;
        if (copy_len > len) copy_len = len;
        
        memcpy(ctx->buffer + ctx->buffer_len, data, copy_len);
        ctx->buffer_len += copy_len;
        data += copy_len;
        len -= copy_len;
        
        if (ctx->buffer_len == SM3_BLOCK_SIZE) {
            sm3_compress(ctx->state, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
    
    // 处理完整的数据块
    while (len >= SM3_BLOCK_SIZE) {
        sm3_compress(ctx->state, data);
        data += SM3_BLOCK_SIZE;
        len -= SM3_BLOCK_SIZE;
    }
    
    // 保存剩余数据到缓冲区
    if (len > 0) {
        memcpy(ctx->buffer, data, len);
        ctx->buffer_len = len;
    }
}

// 完成SM3哈希计算
void sm3_final(sm3_ctx_t *ctx, uint8_t *digest) {
    if (!ctx || !digest) return;
    
    // 添加填充
    ctx->buffer[ctx->buffer_len++] = 0x80;
    
    if (ctx->buffer_len > SM3_BLOCK_SIZE - 8) {
        memset(ctx->buffer + ctx->buffer_len, 0, SM3_BLOCK_SIZE - ctx->buffer_len);
        sm3_compress(ctx->state, ctx->buffer);
        ctx->buffer_len = 0;
    }
    
    memset(ctx->buffer + ctx->buffer_len, 0, SM3_BLOCK_SIZE - ctx->buffer_len - 8);
    
    // 添加消息长度（以位为单位）
    uint64_t bit_count = ctx->count * 8;
    for (int i = 0; i < 8; i++) {
        ctx->buffer[SM3_BLOCK_SIZE - 8 + i] = (uint8_t)(bit_count >> (56 - i * 8));
    }
    
    sm3_compress(ctx->state, ctx->buffer);
    
    // 输出结果（大端序）
    for (int i = 0; i < SM3_STATE_SIZE; i++) {
        digest[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        digest[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
}

// 便捷函数：直接计算哈希
void sm3_hash(const uint8_t *data, size_t len, uint8_t *digest) {
    sm3_ctx_t ctx;
    sm3_init(&ctx);
    sm3_update(&ctx, data, len);
    sm3_final(&ctx, digest);
}

// 便捷函数：计算字符串哈希
void sm3_hash_string(const char *str, uint8_t *digest) {
    if (!str) return;
    sm3_hash((const uint8_t*)str, strlen(str), digest);
}

// 打印摘要
void sm3_print_digest(const uint8_t *digest) {
    if (!digest) return;
    
    printf("SM3 Digest: ");
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");
}

// 验证摘要
int sm3_verify_digest(const uint8_t *digest1, const uint8_t *digest2) {
    if (!digest1 || !digest2) return 0;
    return memcmp(digest1, digest2, SM3_DIGEST_SIZE) == 0;
}

// 提取状态信息（用于长度扩展攻击）
void sm3_extract_state(const sm3_ctx_t *ctx, sm3_state_info_t *state_info) {
    if (!ctx || !state_info) return;
    
    // 将状态转换为字节数组
    for (int i = 0; i < SM3_STATE_SIZE; i++) {
        state_info->digest[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        state_info->digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        state_info->digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        state_info->digest[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
    state_info->message_length = ctx->count;
}

// 长度扩展攻击实现
int sm3_length_extension_attack(const uint8_t *original_digest, 
                                uint64_t original_length,
                                const uint8_t *extension_data, 
                                size_t extension_len,
                                uint8_t *new_digest) {
    if (!original_digest || !extension_data || !new_digest) return 0;
    
    // 将原始摘要转换为状态
    sm3_ctx_t ctx;
    for (int i = 0; i < SM3_STATE_SIZE; i++) {
        ctx.state[i] = ((uint32_t)original_digest[i * 4] << 24) |
                       ((uint32_t)original_digest[i * 4 + 1] << 16) |
                       ((uint32_t)original_digest[i * 4 + 2] << 8) |
                       ((uint32_t)original_digest[i * 4 + 3]);
    }
    
    // 设置消息长度计数
    ctx.count = original_length;
    ctx.buffer_len = 0;
    
    // 添加扩展数据
    sm3_update(&ctx, extension_data, extension_len);
    sm3_final(&ctx, new_digest);
    
    return 1;
}
