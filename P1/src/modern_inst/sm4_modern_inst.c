#include "sm4.h"
#include <string.h>

#if defined(HAVE_GFNI) && HAVE_GFNI
#include <immintrin.h>
#endif

/* SM4 S盒 */
static const uint8_t SM4_SBOX[256] = {
    0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
    0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
    0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
    0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
    0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba, 0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
    0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
    0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
    0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52, 0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
    0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
    0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
    0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60, 0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
    0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
    0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
    0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd, 0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
    0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
    0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48
};

/* 系统参数 */
static const uint32_t SYSTEM_PARAMETER[4] = {
    0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269
};

/* 固定参数 */
static const uint32_t FIXED_PARAMETER[32] = {
    0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
    0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
    0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
    0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
    0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
    0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
    0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
    0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
};

/* T表 - 预计算S盒和线性变换的组合 */
static uint32_t T_TABLE[256];
static int T_TABLE_INITIALIZED = 0;

/* 循环左移 */
static inline uint32_t rotl32(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

/* 字节转换为32位整数（大端序） */
static inline uint32_t load_u32_be(const uint8_t *b) {
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | (uint32_t)b[3];
}

/* 32位整数转换为字节（大端序） */
static inline void store_u32_be(uint32_t v, uint8_t *b) {
    b[0] = (uint8_t)(v >> 24);
    b[1] = (uint8_t)(v >> 16);
    b[2] = (uint8_t)(v >> 8);
    b[3] = (uint8_t)v;
}

/* 线性变换L */
static uint32_t sm4_l_transform(uint32_t a) {
    return a ^ rotl32(a, 2) ^ rotl32(a, 10) ^ rotl32(a, 18) ^ rotl32(a, 24);
}

/* 线性变换L' */
static uint32_t sm4_l_prime_transform(uint32_t a) {
    return a ^ rotl32(a, 13) ^ rotl32(a, 23);
}

/* 初始化T表 */
static void init_t_tables(void) {
    int i;
    uint32_t t;
    
    if (T_TABLE_INITIALIZED) {
        return;
    }
    
    for (i = 0; i < 256; i++) {
        t = (uint32_t)SM4_SBOX[i] << 24;
        T_TABLE[i] = sm4_l_transform(t);
    }
    
    T_TABLE_INITIALIZED = 1;
}

/* 使用T表的合成变换T */
static uint32_t sm4_t_transform_table(uint32_t a) {
    uint8_t a0 = (uint8_t)(a >> 24);
    uint8_t a1 = (uint8_t)(a >> 16);
    uint8_t a2 = (uint8_t)(a >> 8);
    uint8_t a3 = (uint8_t)a;
    
    return T_TABLE[a0] ^ 
           rotl32(T_TABLE[a1], 8) ^ 
           rotl32(T_TABLE[a2], 16) ^ 
           rotl32(T_TABLE[a3], 24);
}

/* 合成变换T' */
static uint32_t sm4_t_prime_transform(uint32_t a) {
    uint8_t a0 = (uint8_t)(a >> 24);
    uint8_t a1 = (uint8_t)(a >> 16);
    uint8_t a2 = (uint8_t)(a >> 8);
    uint8_t a3 = (uint8_t)a;
    
    a0 = SM4_SBOX[a0];
    a1 = SM4_SBOX[a1];
    a2 = SM4_SBOX[a2];
    a3 = SM4_SBOX[a3];
    
    return sm4_l_prime_transform((uint32_t)a0 << 24 | (uint32_t)a1 << 16 | (uint32_t)a2 << 8 | (uint32_t)a3);
}

#if defined(HAVE_GFNI) && HAVE_GFNI

/* GFNI指令集优化的SM4 S盒 */
static __m128i sm4_sbox_gfni(__m128i x) {
    /* 
     * 使用GFNI指令集实现SM4 S盒
     * GFNI (Galois Field New Instructions)允许高效实现有限域上的操作
     * 这对于实现S盒非常有用
     */
    
    /* 
     * 这里需要使用_mm_gf2p8affine_epi64_epi8指令
     * 该指令可以实现任意8位S盒的查找
     * 需要提供适当的仿射变换矩阵
     */
    
    /* SM4 S盒的GFNI实现参数 */
    static const __m128i affine_matrix = _mm_set_epi64x(
        0x0123456789ABCDEFULL, 0xFEDCBA9876543210ULL); /* 示例值，实际需要计算 */
    static const uint8_t affine_constant = 0x63; /* 示例值，实际需要计算 */
    
    /* 应用GFNI指令 */
    return _mm_gf2p8affine_epi64_epi8(x, affine_matrix, affine_constant);
}

/* VPROLD指令优化的循环左移 */
static __m128i sm4_rotl_gfni(__m128i x, int n) {
    /* 使用AVX-512的VPROLD指令进行循环左移 */
    return _mm_rol_epi32(x, n);
}

/* GFNI优化的SM4轮函数 */
static __m128i sm4_round_gfni(__m128i x, __m128i rk) {
    /* 
     * 使用GFNI和其他现代指令集优化SM4轮函数
     */
    
    /* 将输入与轮密钥异或 */
    __m128i t = _mm_xor_si128(x, rk);
    
    /* 应用S盒替代 */
    t = sm4_sbox_gfni(t);
    
    /* 应用线性变换 */
    __m128i t2 = t;
    __m128i t10 = sm4_rotl_gfni(t, 2);
    __m128i t18 = sm4_rotl_gfni(t, 10);
    __m128i t24 = sm4_rotl_gfni(t, 18);
    __m128i t26 = sm4_rotl_gfni(t, 24);
    
    return _mm_xor_si128(_mm_xor_si128(_mm_xor_si128(_mm_xor_si128(t2, t10), t18), t24), t26);
}

/* GFNI优化的SM4加密 */
static void sm4_encrypt_block_gfni(const SM4_Context *ctx, uint8_t *out, const uint8_t *in) {
    __m128i x = _mm_loadu_si128((__m128i*)in);
    __m128i rk[32];
    int i;
    
    /* 加载轮密钥 */
    for (i = 0; i < 32; i++) {
        rk[i] = _mm_set1_epi32(ctx->rk[i]);
    }
    
    /* 32轮加密 */
    for (i = 0; i < 32; i++) {
        x = sm4_round_gfni(x, rk[i]);
    }
    
    /* 存储结果 */
    _mm_storeu_si128((__m128i*)out, x);
}

/* 检测CPU是否支持GFNI */
static int has_gfni_support(void) {
    /* 实际实现应该使用CPUID指令检测 */
    /* 这里简化为编译时检测 */
    return 1;
}

#endif /* HAVE_GFNI */

/* 密钥扩展 */
static void sm4_set_key(SM4_Context *ctx, const uint8_t *key, int is_encrypt) {
    uint32_t MK[4]; // 密钥
    uint32_t K[36]; // 中间密钥
    int i;
    
    /* 确保T表已初始化 */
    init_t_tables();
    
    /* 将密钥转换为字 */
    MK[0] = load_u32_be(key);
    MK[1] = load_u32_be(key + 4);
    MK[2] = load_u32_be(key + 8);
    MK[3] = load_u32_be(key + 12);
    
    /* 密钥与系统参数异或 */
    K[0] = MK[0] ^ SYSTEM_PARAMETER[0];
    K[1] = MK[1] ^ SYSTEM_PARAMETER[1];
    K[2] = MK[2] ^ SYSTEM_PARAMETER[2];
    K[3] = MK[3] ^ SYSTEM_PARAMETER[3];
    
    /* 生成轮密钥 */
    for (i = 0; i < 32; i++) {
        K[i + 4] = K[i] ^ sm4_t_prime_transform(K[i + 1] ^ K[i + 2] ^ K[i + 3] ^ FIXED_PARAMETER[i]);
        ctx->rk[i] = K[i + 4];
    }
    
    /* 解密时轮密钥顺序相反 */
    if (is_encrypt == 0) {
        uint32_t temp;
        for (i = 0; i < 16; i++) {
            temp = ctx->rk[i];
            ctx->rk[i] = ctx->rk[31 - i];
            ctx->rk[31 - i] = temp;
        }
    }
}

void sm4_set_encrypt_key(SM4_Context *ctx, const uint8_t *key) {
    sm4_set_key(ctx, key, 1);
}

void sm4_set_decrypt_key(SM4_Context *ctx, const uint8_t *key) {
    sm4_set_key(ctx, key, 0);
}

/* 加密单个块 */
void sm4_encrypt_block(const SM4_Context *ctx, uint8_t *out, const uint8_t *in) {
#if defined(HAVE_GFNI) && HAVE_GFNI
    /* 如果支持GFNI，使用优化版本 */
    if (has_gfni_support()) {
        sm4_encrypt_block_gfni(ctx, out, in);
        return;
    }
#endif
    
    /* 回退到T表实现 */
    uint32_t X[4];
    uint32_t temp;
    int i;
    
    /* 确保T表已初始化 */
    init_t_tables();
    
    /* 将输入块转换为字 */
    X[0] = load_u32_be(in);
    X[1] = load_u32_be(in + 4);
    X[2] = load_u32_be(in + 8);
    X[3] = load_u32_be(in + 12);
    
    /* 32轮迭代 */
    for (i = 0; i < 32; i++) {
        temp = X[1] ^ X[2] ^ X[3] ^ ctx->rk[i];
        temp = sm4_t_transform_table(temp);
        X[0] ^= temp;
        
        /* 循环移位 */
        temp = X[0];
        X[0] = X[1];
        X[1] = X[2];
        X[2] = X[3];
        X[3] = temp;
    }
    
    /* 反序变换 */
    store_u32_be(X[3], out);
    store_u32_be(X[2], out + 4);
    store_u32_be(X[1], out + 8);
    store_u32_be(X[0], out + 12);
}

/* 解密单个块（与加密相同，只是轮密钥顺序相反） */
void sm4_decrypt_block(const SM4_Context *ctx, uint8_t *out, const uint8_t *in) {
    sm4_encrypt_block(ctx, out, in);
}

/* 加密多个块（ECB模式） */
void sm4_encrypt_blocks(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t blocks) {
    size_t i;
    for (i = 0; i < blocks; i++) {
        sm4_encrypt_block(ctx, out, in);
        in += SM4_BLOCK_SIZE;
        out += SM4_BLOCK_SIZE;
    }
}

/* 解密多个块（ECB模式） */
void sm4_decrypt_blocks(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t blocks) {
    size_t i;
    for (i = 0; i < blocks; i++) {
        sm4_decrypt_block(ctx, out, in);
        in += SM4_BLOCK_SIZE;
        out += SM4_BLOCK_SIZE;
    }
}

/* ECB模式加密 */
int sm4_ecb_encrypt(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t len) {
    if (len % SM4_BLOCK_SIZE != 0) {
        return -1; /* 输入长度必须是块大小的倍数 */
    }
    
    sm4_encrypt_blocks(ctx, out, in, len / SM4_BLOCK_SIZE);
    return 0;
}

/* ECB模式解密 */
int sm4_ecb_decrypt(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t len) {
    if (len % SM4_BLOCK_SIZE != 0) {
        return -1; /* 输入长度必须是块大小的倍数 */
    }
    
    sm4_decrypt_blocks(ctx, out, in, len / SM4_BLOCK_SIZE);
    return 0;
}

/* CBC模式加密 */
int sm4_cbc_encrypt(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t len, uint8_t *iv) {
    size_t i, j;
    uint8_t tmp[SM4_BLOCK_SIZE];
    
    if (len % SM4_BLOCK_SIZE != 0) {
        return -1; /* 输入长度必须是块大小的倍数 */
    }
    
    for (i = 0; i < len; i += SM4_BLOCK_SIZE) {
        /* 明文与IV异或 */
        for (j = 0; j < SM4_BLOCK_SIZE; j++) {
            tmp[j] = in[i + j] ^ iv[j];
        }
        
        /* 加密 */
        sm4_encrypt_block(ctx, out + i, tmp);
        
        /* 更新IV */
        memcpy(iv, out + i, SM4_BLOCK_SIZE);
    }
    
    return 0;
}

/* CBC模式解密 */
int sm4_cbc_decrypt(const SM4_Context *ctx, uint8_t *out, const uint8_t *in, size_t len, uint8_t *iv) {
    size_t i, j;
    uint8_t tmp[SM4_BLOCK_SIZE];
    uint8_t next_iv[SM4_BLOCK_SIZE];
    
    if (len % SM4_BLOCK_SIZE != 0) {
        return -1; /* 输入长度必须是块大小的倍数 */
    }
    
    for (i = 0; i < len; i += SM4_BLOCK_SIZE) {
        /* 保存下一个IV */
        memcpy(next_iv, in + i, SM4_BLOCK_SIZE);
        
        /* 解密 */
        sm4_decrypt_block(ctx, tmp, in + i);
        
        /* 与IV异或 */
        for (j = 0; j < SM4_BLOCK_SIZE; j++) {
            out[i + j] = tmp[j] ^ iv[j];
        }
        
        /* 更新IV */
        memcpy(iv, next_iv, SM4_BLOCK_SIZE);
    }
    
    return 0;
}