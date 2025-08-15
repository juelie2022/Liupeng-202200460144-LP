#ifndef SM4_GCM_H
#define SM4_GCM_H

#include "sm4.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SM4-GCM 上下文结构 */
typedef struct {
    SM4_Context cipher_ctx;  // SM4 上下文
    uint8_t H[SM4_BLOCK_SIZE];  // GHASH密钥
    uint8_t J0[SM4_BLOCK_SIZE]; // 初始计数器
    uint64_t len_a;  // 附加数据长度
    uint64_t len_c;  // 密文长度
    uint8_t buf[SM4_BLOCK_SIZE]; // 部分块缓冲区
    size_t buf_len;  // 缓冲区中的字节数
    uint8_t final_ghash[SM4_BLOCK_SIZE]; // 最终GHASH值
} SM4_GCM_Context;

/**
 * @brief 初始化SM4-GCM上下文
 * @param ctx GCM上下文
 * @param key 16字节密钥
 * @param iv IV/Nonce
 * @param iv_len IV长度（字节）
 * @return 0成功，非0失败
 */
int sm4_gcm_init(SM4_GCM_Context *ctx, const uint8_t *key, const uint8_t *iv, size_t iv_len);

/**
 * @brief 处理附加认证数据(AAD)
 * @param ctx GCM上下文
 * @param aad 附加数据
 * @param aad_len 附加数据长度（字节）
 * @return 0成功，非0失败
 */
int sm4_gcm_aad(SM4_GCM_Context *ctx, const uint8_t *aad, size_t aad_len);

/**
 * @brief SM4-GCM加密
 * @param ctx GCM上下文
 * @param out 输出密文
 * @param in 输入明文
 * @param len 明文长度（字节）
 * @return 0成功，非0失败
 */
int sm4_gcm_encrypt(SM4_GCM_Context *ctx, uint8_t *out, const uint8_t *in, size_t len);

/**
 * @brief SM4-GCM解密
 * @param ctx GCM上下文
 * @param out 输出明文
 * @param in 输入密文
 * @param len 密文长度（字节）
 * @return 0成功，非0失败
 */
int sm4_gcm_decrypt(SM4_GCM_Context *ctx, uint8_t *out, const uint8_t *in, size_t len);

/**
 * @brief 完成SM4-GCM操作并生成认证标签
 * @param ctx GCM上下文
 * @param tag 输出认证标签
 * @param tag_len 标签长度（字节），通常为16
 * @return 0成功，非0失败
 */
int sm4_gcm_finish(SM4_GCM_Context *ctx, uint8_t *tag, size_t tag_len);

/**
 * @brief 一步完成SM4-GCM加密和认证
 * @param key 16字节密钥
 * @param iv IV/Nonce
 * @param iv_len IV长度（字节）
 * @param aad 附加数据
 * @param aad_len 附加数据长度（字节）
 * @param in 输入明文
 * @param in_len 明文长度（字节）
 * @param out 输出密文
 * @param tag 输出认证标签
 * @param tag_len 标签长度（字节），通常为16
 * @return 0成功，非0失败
 */
int sm4_gcm_encrypt_and_tag(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                           const uint8_t *aad, size_t aad_len,
                           const uint8_t *in, size_t in_len,
                           uint8_t *out, uint8_t *tag, size_t tag_len);

/**
 * @brief 一步完成SM4-GCM解密和验证
 * @param key 16字节密钥
 * @param iv IV/Nonce
 * @param iv_len IV长度（字节）
 * @param aad 附加数据
 * @param aad_len 附加数据长度（字节）
 * @param in 输入密文
 * @param in_len 密文长度（字节）
 * @param tag 输入认证标签
 * @param tag_len 标签长度（字节），通常为16
 * @param out 输出明文
 * @return 0成功（验证通过），非0失败（验证失败）
 */
int sm4_gcm_decrypt_and_verify(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                              const uint8_t *aad, size_t aad_len,
                              const uint8_t *in, size_t in_len,
                              const uint8_t *tag, size_t tag_len,
                              uint8_t *out);

#ifdef __cplusplus
}
#endif

#endif /* SM4_GCM_H */