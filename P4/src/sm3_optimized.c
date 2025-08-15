#include "sm3.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>

// 优化版本的常量表（预计算）
static uint32_t SM3_T_OPTIMIZED[64];
static uint32_t SM3_ROTATION_TABLE[64];

// 初始化优化表
static void init_optimization_tables() {
    static int initialized = 0;
    if (initialized) return;
    
    // 预计算T值
    for (int i = 0; i < 64; i++) {
        if (i < 16) {
            SM3_T_OPTIMIZED[i] = 0x79CC4519;
        } else {
            SM3_T_OPTIMIZED[i] = 0x7A879D8A;
        }
        SM3_ROTATION_TABLE[i] = i;
    }
    initialized = 1;
}

// 优化的左循环移位（内联）
static inline uint32_t ROTL_OPT(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

// 优化的基本函数（内联）
static inline uint32_t FF_OPT(uint32_t x, uint32_t y, uint32_t z, int j) {
    return (j < 16) ? (x ^ y ^ z) : ((x & y) | (x & z) | (y & z));
}

static inline uint32_t GG_OPT(uint32_t x, uint32_t y, uint32_t z, int j) {
    return (j < 16) ? (x ^ y ^ z) : ((x & y) | (~x & z));
}

static inline uint32_t P0_OPT(uint32_t x) {
    return x ^ ROTL_OPT(x, 9) ^ ROTL_OPT(x, 17);
}

static inline uint32_t P1_OPT(uint32_t x) {
    return x ^ ROTL_OPT(x, 15) ^ ROTL_OPT(x, 23);
}

// 优化的消息扩展（循环展开）
static void expand_message_optimized(uint32_t W[68], uint32_t W1[64], const uint8_t block[64]) {
    // 前16个字（循环展开）
    W[0] = ((uint32_t)block[0] << 24) | ((uint32_t)block[1] << 16) | ((uint32_t)block[2] << 8) | block[3];
    W[1] = ((uint32_t)block[4] << 24) | ((uint32_t)block[5] << 16) | ((uint32_t)block[6] << 8) | block[7];
    W[2] = ((uint32_t)block[8] << 24) | ((uint32_t)block[9] << 16) | ((uint32_t)block[10] << 8) | block[11];
    W[3] = ((uint32_t)block[12] << 24) | ((uint32_t)block[13] << 16) | ((uint32_t)block[14] << 8) | block[15];
    W[4] = ((uint32_t)block[16] << 24) | ((uint32_t)block[17] << 16) | ((uint32_t)block[18] << 8) | block[19];
    W[5] = ((uint32_t)block[20] << 24) | ((uint32_t)block[21] << 16) | ((uint32_t)block[22] << 8) | block[23];
    W[6] = ((uint32_t)block[24] << 24) | ((uint32_t)block[25] << 16) | ((uint32_t)block[26] << 8) | block[27];
    W[7] = ((uint32_t)block[28] << 24) | ((uint32_t)block[29] << 16) | ((uint32_t)block[30] << 8) | block[31];
    W[8] = ((uint32_t)block[32] << 24) | ((uint32_t)block[33] << 16) | ((uint32_t)block[34] << 8) | block[35];
    W[9] = ((uint32_t)block[36] << 24) | ((uint32_t)block[37] << 16) | ((uint32_t)block[38] << 8) | block[39];
    W[10] = ((uint32_t)block[40] << 24) | ((uint32_t)block[41] << 16) | ((uint32_t)block[42] << 8) | block[43];
    W[11] = ((uint32_t)block[44] << 24) | ((uint32_t)block[45] << 16) | ((uint32_t)block[46] << 8) | block[47];
    W[12] = ((uint32_t)block[48] << 24) | ((uint32_t)block[49] << 16) | ((uint32_t)block[50] << 8) | block[51];
    W[13] = ((uint32_t)block[52] << 24) | ((uint32_t)block[53] << 16) | ((uint32_t)block[54] << 8) | block[55];
    W[14] = ((uint32_t)block[56] << 24) | ((uint32_t)block[57] << 16) | ((uint32_t)block[58] << 8) | block[59];
    W[15] = ((uint32_t)block[60] << 24) | ((uint32_t)block[61] << 16) | ((uint32_t)block[62] << 8) | block[63];
    
    // 扩展计算（部分循环展开）
    for (int j = 16; j < 68; j += 4) {
        if (j + 3 < 68) {
            W[j] = P1_OPT(W[j - 16] ^ W[j - 9] ^ ROTL_OPT(W[j - 3], 15)) ^ 
                    ROTL_OPT(W[j - 13], 7) ^ W[j - 6];
            W[j + 1] = P1_OPT(W[j - 15] ^ W[j - 8] ^ ROTL_OPT(W[j - 2], 15)) ^ 
                        ROTL_OPT(W[j - 12], 7) ^ W[j - 5];
            W[j + 2] = P1_OPT(W[j - 14] ^ W[j - 7] ^ ROTL_OPT(W[j - 1], 15)) ^ 
                        ROTL_OPT(W[j - 11], 7) ^ W[j - 4];
            W[j + 3] = P1_OPT(W[j - 13] ^ W[j - 6] ^ ROTL_OPT(W[j], 15)) ^ 
                        ROTL_OPT(W[j - 10], 7) ^ W[j - 3];
        } else {
            for (int k = j; k < 68; k++) {
                W[k] = P1_OPT(W[k - 16] ^ W[k - 9] ^ ROTL_OPT(W[k - 3], 15)) ^ 
                        ROTL_OPT(W[k - 13], 7) ^ W[k - 6];
            }
            break;
        }
    }
    
    // W1计算（循环展开）
    for (int j = 0; j < 64; j += 4) {
        if (j + 3 < 64) {
            W1[j] = W[j] ^ W[j + 4];
            W1[j + 1] = W[j + 1] ^ W[j + 5];
            W1[j + 2] = W[j + 2] ^ W[j + 6];
            W1[j + 3] = W[j + 3] ^ W[j + 7];
        } else {
            for (int k = j; k < 64; k++) {
                W1[k] = W[k] ^ W[k + 4];
            }
            break;
        }
    }
}

// 优化的压缩函数（主循环展开）
static void sm3_compress_optimized(uint32_t state[8], const uint8_t block[64]) {
    uint32_t W[68];
    uint32_t W1[64];
    uint32_t A, B, C, D, E, F, G, H;
    uint32_t SS1, SS2, TT1, TT2;
    
    // 初始化优化表
    init_optimization_tables();
    
    // 消息扩展
    expand_message_optimized(W, W1, block);
    
    // 初始化工作变量
    A = state[0];
    B = state[1];
    C = state[2];
    D = state[3];
    E = state[4];
    F = state[5];
    G = state[6];
    H = state[7];
    
    // 主循环（部分展开）
    for (int j = 0; j < 64; j += 4) {
        if (j + 3 < 64) {
            // 第j轮
            SS1 = ROTL_OPT((ROTL_OPT(A, 12) + E + ROTL_OPT(SM3_T_OPTIMIZED[j], j)), 7);
            SS2 = SS1 ^ ROTL_OPT(A, 12);
            TT1 = FF_OPT(A, B, C, j) + D + SS2 + W1[j];
            TT2 = GG_OPT(E, F, G, j) + H + SS1 + W[j];
            D = C;
            C = ROTL_OPT(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = ROTL_OPT(F, 19);
            F = E;
            E = P0_OPT(TT2);
            
            // 第j+1轮
            SS1 = ROTL_OPT((ROTL_OPT(A, 12) + E + ROTL_OPT(SM3_T_OPTIMIZED[j + 1], j + 1)), 7);
            SS2 = SS1 ^ ROTL_OPT(A, 12);
            TT1 = FF_OPT(A, B, C, j + 1) + D + SS2 + W1[j + 1];
            TT2 = GG_OPT(E, F, G, j + 1) + H + SS1 + W[j + 1];
            D = C;
            C = ROTL_OPT(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = ROTL_OPT(F, 19);
            F = E;
            E = P0_OPT(TT2);
            
            // 第j+2轮
            SS1 = ROTL_OPT((ROTL_OPT(A, 12) + E + ROTL_OPT(SM3_T_OPTIMIZED[j + 2], j + 2)), 7);
            SS2 = SS1 ^ ROTL_OPT(A, 12);
            TT1 = FF_OPT(A, B, C, j + 2) + D + SS2 + W1[j + 2];
            TT2 = GG_OPT(E, F, G, j + 2) + H + SS1 + W[j + 2];
            D = C;
            C = ROTL_OPT(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = ROTL_OPT(F, 19);
            F = E;
            E = P0_OPT(TT2);
            
            // 第j+3轮
            SS1 = ROTL_OPT((ROTL_OPT(A, 12) + E + ROTL_OPT(SM3_T_OPTIMIZED[j + 3], j + 3)), 7);
            SS2 = SS1 ^ ROTL_OPT(A, 12);
            TT1 = FF_OPT(A, B, C, j + 3) + D + SS2 + W1[j + 3];
            TT2 = GG_OPT(E, F, G, j + 3) + H + SS1 + W[j + 3];
            D = C;
            C = ROTL_OPT(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = ROTL_OPT(F, 19);
            F = E;
            E = P0_OPT(TT2);
        } else {
            // 处理剩余轮次
            for (int k = j; k < 64; k++) {
                SS1 = ROTL_OPT((ROTL_OPT(A, 12) + E + ROTL_OPT(SM3_T_OPTIMIZED[k], k)), 7);
                SS2 = SS1 ^ ROTL_OPT(A, 12);
                TT1 = FF_OPT(A, B, C, k) + D + SS2 + W1[k];
                TT2 = GG_OPT(E, F, G, k) + H + SS1 + W[k];
                D = C;
                C = ROTL_OPT(B, 9);
                B = A;
                A = TT1;
                H = G;
                G = ROTL_OPT(F, 19);
                F = E;
                E = P0_OPT(TT2);
            }
            break;
        }
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

// 优化的初始化函数
void sm3_init_optimized(sm3_ctx_t *ctx) {
    if (!ctx) return;
    
    // 初始化优化表
    init_optimization_tables();
    
    memcpy(ctx->state, SM3_IV, sizeof(SM3_IV));
    ctx->count = 0;
    ctx->buffer_len = 0;
    memset(ctx->buffer, 0, SM3_BLOCK_SIZE);
}

// 优化的更新函数
void sm3_update_optimized(sm3_ctx_t *ctx, const uint8_t *data, size_t len) {
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
            sm3_compress_optimized(ctx->state, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
    
    // 处理完整的数据块（向量化处理）
    while (len >= SM3_BLOCK_SIZE) {
        sm3_compress_optimized(ctx->state, data);
        data += SM3_BLOCK_SIZE;
        len -= SM3_BLOCK_SIZE;
    }
    
    // 保存剩余数据到缓冲区
    if (len > 0) {
        memcpy(ctx->buffer, data, len);
        ctx->buffer_len = len;
    }
}

// 优化的完成函数
void sm3_final_optimized(sm3_ctx_t *ctx, uint8_t *digest) {
    if (!ctx || !digest) return;
    
    // 添加填充
    ctx->buffer[ctx->buffer_len++] = 0x80;
    
    if (ctx->buffer_len > SM3_BLOCK_SIZE - 8) {
        memset(ctx->buffer + ctx->buffer_len, 0, SM3_BLOCK_SIZE - ctx->buffer_len);
        sm3_compress_optimized(ctx->state, ctx->buffer);
        ctx->buffer_len = 0;
    }
    
    memset(ctx->buffer + ctx->buffer_len, 0, SM3_BLOCK_SIZE - ctx->buffer_len - 8);
    
    // 添加消息长度（以位为单位）
    uint64_t bit_count = ctx->count * 8;
    for (int i = 0; i < 8; i++) {
        ctx->buffer[SM3_BLOCK_SIZE - 8 + i] = (uint8_t)(bit_count >> (56 - i * 8));
    }
    
    sm3_compress_optimized(ctx->state, ctx->buffer);
    
    // 输出结果（大端序）
    for (int i = 0; i < SM3_STATE_SIZE; i++) {
        digest[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        digest[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
}

// 优化的便捷函数
void sm3_hash_optimized(const uint8_t *data, size_t len, uint8_t *digest) {
    sm3_ctx_t ctx;
    sm3_init_optimized(&ctx);
    sm3_update_optimized(&ctx, data, len);
    sm3_final_optimized(&ctx, digest);
}

// 性能测试函数
void sm3_benchmark(size_t data_size, int iterations) {
    uint8_t *data = (uint8_t*)safe_malloc(data_size);
    uint8_t digest[SM3_DIGEST_SIZE];
    timer_t timer;
    double total_time = 0.0;
    
    if (!data) return;
    
    // 生成随机数据
    random_bytes(data, data_size);
    
    printf("SM3性能测试: %zu 字节, %d 次迭代\n", data_size, iterations);
    
    // 测试基本版本
    timer_start(&timer);
    for (int i = 0; i < iterations; i++) {
        sm3_hash(data, data_size, digest);
    }
    timer_stop(&timer);
    double basic_time = timer_get_elapsed_ms(&timer);
    
    // 测试优化版本
    timer_start(&timer);
    for (int i = 0; i < iterations; i++) {
        sm3_hash_optimized(data, data_size, digest);
    }
    timer_stop(&timer);
    double optimized_time = timer_get_elapsed_ms(&timer);
    
    printf("基本版本: %.3f ms (%.2f MB/s)\n", 
           basic_time, (data_size * iterations / 1024.0 / 1024.0) / (basic_time / 1000.0));
    printf("优化版本: %.3f ms (%.2f MB/s)\n", 
           optimized_time, (data_size * iterations / 1024.0 / 1024.0) / (optimized_time / 1000.0));
    printf("性能提升: %.2fx\n", basic_time / optimized_time);
    
    safe_free(data);
}

// 性能测量函数
double sm3_measure_performance(size_t data_size, int iterations) {
    uint8_t *data = (uint8_t*)safe_malloc(data_size);
    uint8_t digest[SM3_DIGEST_SIZE];
    timer_t timer;
    double total_time = 0.0;
    
    if (!data) return 0.0;
    
    random_bytes(data, data_size);
    
    timer_start(&timer);
    for (int i = 0; i < iterations; i++) {
        sm3_hash_optimized(data, data_size, digest);
    }
    timer_stop(&timer);
    
    total_time = timer_get_elapsed_ms(&timer);
    safe_free(data);
    
    return total_time;
}
