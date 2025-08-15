#ifndef SM4_CPU_FEATURES_H
#define SM4_CPU_FEATURES_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CPU特性检测结果
 */
typedef struct {
    bool has_aesni;    // 支持AES-NI指令集
    bool has_sse2;     // 支持SSE2指令集
    bool has_avx;      // 支持AVX指令集
    bool has_avx2;     // 支持AVX2指令集
    bool has_avx512f;  // 支持AVX-512 Foundation
    bool has_gfni;     // 支持GFNI指令集
    bool has_vaes;     // 支持向量化AES指令
    bool has_vpclmulqdq; // 支持向量化PCLMULQDQ指令
} SM4_CPU_Features;

/**
 * @brief 检测CPU支持的指令集特性
 * @return 包含CPU特性的结构体
 */
SM4_CPU_Features sm4_get_cpu_features(void);

/**
 * @brief 获取最优的SM4实现方式
 * @return 实现类型的字符串描述
 */
const char* sm4_get_best_implementation(void);

/**
 * @brief 强制使用特定的SM4实现
 * @param impl_name 实现名称，可以是"basic", "t_table", "aesni", "gfni"等
 * @return 0成功，非0失败
 */
int sm4_force_implementation(const char* impl_name);

#ifdef __cplusplus
}
#endif

#endif /* SM4_CPU_FEATURES_H */