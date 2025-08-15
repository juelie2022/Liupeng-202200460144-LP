#ifndef SM4_H
#define SM4_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SM4 常量定义 */
#define SM4_BLOCK_SIZE 16  // 块大小（字节）
#define SM4_KEY_SIZE 16    // 密钥大小（字节）
#define SM4_ROUNDS 32      // 轮数

/* SM4 上下文结构 */
typedef struct {
    uint32_t rk[SM4_ROUNDS];  // 轮密钥
} SM4_Context;

/* 基本SM4函数 */

/**
 * @brief 初始化SM4加密上下文
 * @param ctx SM4上下文
 * @param key 16字节密钥
 */
void sm4_set_encrypt_key(SM4_Context *ctx, const uint8_t *key);

/**
 * @brief 初始化SM4解密上下文
 * @param ctx SM4上下文
 * @param key 16字节密钥
 */
void sm4_set_decrypt_key(SM4_Context *ctx, const uint8_t *key);

/**
 * @brief 加密单个数据块
 * @param ctx SM4上下文
 * @param out 输出密文（16字节）
 * @param in 输入明文（16字节）
 */
void sm4_encrypt_block(const SM4_Context *ctx, uint8_t *out, const uint8_t *in);

/**
 * @brief 解密单个数据块
 * @param ctx SM4上下文
 * @param out 输出明文（16字节）
 * @param in 输入密文（16字节）
 */
void sm4_decrypt_block(const SM4_Context *ctx, uint8_t *out, const uint8_t *in);

/**
 * @brief 加密多个数据块（ECB模式）
 * @param ctx SM4上下文
 * @param out 输出密文
 * @param in 输入明文
 * @param blocks 块数量
 */
void sm4_encrypt_blocks(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t blocks);

/**
 * @brief 解密多个数据块（ECB模式）
 * @param ctx SM4上下文
 * @param out 输出明文
 * @param in 输入密文
 * @param blocks 块数量
 */
void sm4_decrypt_blocks(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t blocks);

/* 工作模式 */

/**
 * @brief SM4-ECB模式加密
 * @param ctx SM4上下文
 * @param out 输出密文
 * @param in 输入明文
 * @param len 明文长度（必须是16的倍数）
 * @return 0成功，非0失败
 */
int sm4_ecb_encrypt(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t len);

/**
 * @brief SM4-ECB模式解密
 * @param ctx SM4上下文
 * @param out 输出明文
 * @param in 输入密文
 * @param len 密文长度（必须是16的倍数）
 * @return 0成功，非0失败
 */
int sm4_ecb_decrypt(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t len);

/**
 * @brief SM4-CBC模式加密
 * @param ctx SM4上下文
 * @param out 输出密文
 * @param in 输入明文
 * @param len 明文长度（必须是16的倍数）
 * @param iv 初始化向量（16字节）
 * @return 0成功，非0失败
 */
int sm4_cbc_encrypt(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t len, uint8_t *iv);

/**
 * @brief SM4-CBC模式解密
 * @param ctx SM4上下文
 * @param out 输出明文
 * @param in 输入密文
 * @param len 密文长度（必须是16的倍数）
 * @param iv 初始化向量（16字节）
 * @return 0成功，非0失败
 */
int sm4_cbc_decrypt(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t len, uint8_t *iv);

#ifdef __cplusplus
}
#endif

#endif /* SM4_H */