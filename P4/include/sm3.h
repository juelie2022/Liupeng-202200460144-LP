#ifndef SM3_H
#define SM3_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// SM3常量定义
#define SM3_BLOCK_SIZE 64
#define SM3_DIGEST_SIZE 32
#define SM3_STATE_SIZE 8

// SM3初始向量
extern const uint32_t SM3_IV[SM3_STATE_SIZE];

// SM3上下文结构
typedef struct {
    uint32_t state[SM3_STATE_SIZE];  // 哈希状态
    uint64_t count;                  // 消息长度计数
    uint8_t buffer[SM3_BLOCK_SIZE];  // 消息缓冲区
    size_t buffer_len;               // 缓冲区中字节数
} sm3_ctx_t;

// 基本SM3函数
void sm3_init(sm3_ctx_t *ctx);
void sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t len);
void sm3_final(sm3_ctx_t *ctx, uint8_t *digest);

// 便捷函数
void sm3_hash(const uint8_t *data, size_t len, uint8_t *digest);
void sm3_hash_string(const char *str, uint8_t *digest);

// 优化版本函数
void sm3_init_optimized(sm3_ctx_t *ctx);
void sm3_update_optimized(sm3_ctx_t *ctx, const uint8_t *data, size_t len);
void sm3_final_optimized(sm3_ctx_t *ctx, uint8_t *digest);
void sm3_hash_optimized(const uint8_t *data, size_t len, uint8_t *digest);

// 性能测试函数
void sm3_benchmark(size_t data_size, int iterations);
double sm3_measure_performance(size_t data_size, int iterations);

// 工具函数
void sm3_print_digest(const uint8_t *digest);
int sm3_verify_digest(const uint8_t *digest1, const uint8_t *digest2);

// 长度扩展攻击相关函数
typedef struct {
    uint8_t digest[SM3_DIGEST_SIZE];
    uint64_t message_length;
} sm3_state_info_t;

void sm3_extract_state(const sm3_ctx_t *ctx, sm3_state_info_t *state_info);
int sm3_length_extension_attack(const uint8_t *original_digest, 
                                uint64_t original_length,
                                const uint8_t *extension_data, 
                                size_t extension_len,
                                uint8_t *new_digest);

#ifdef __cplusplus
}
#endif

#endif // SM3_H
