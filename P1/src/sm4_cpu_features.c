#include "sm4_cpu_features.h"

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif

/* 检测CPU支持的指令集特性 */
SM4_CPU_Features sm4_get_cpu_features(void) {
    SM4_CPU_Features features = {0};

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    /* x86/x64架构 */
    unsigned int eax, ebx, ecx, edx;
    
#if defined(_MSC_VER)
    /* MSVC编译器 */
    int cpu_info[4];
    
    /* 检查基本特性 */
    __cpuid(cpu_info, 1);
    ecx = cpu_info[2];
    edx = cpu_info[3];
    
    features.has_sse2 = (edx >> 26) & 1;
    features.has_aesni = (ecx >> 25) & 1;
    
    /* 检查AVX特性 */
    features.has_avx = (ecx >> 28) & 1;
    
    /* 检查AVX2特性 */
    __cpuid(cpu_info, 7);
    ebx = cpu_info[1];
    features.has_avx2 = (ebx >> 5) & 1;
    
    /* 检查AVX-512特性 */
    features.has_avx512f = (ebx >> 16) & 1;
    
    /* 检查GFNI特性 */
    features.has_gfni = (ecx >> 8) & 1;
    
    /* 检查VAES特性 */
    features.has_vaes = (ecx >> 9) & 1;
    
    /* 检查VPCLMULQDQ特性 */
    features.has_vpclmulqdq = (ecx >> 10) & 1;
    
#else
    /* GCC/Clang编译器 */
    
    /* 检查基本特性 */
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    
    features.has_sse2 = (edx >> 26) & 1;
    features.has_aesni = (ecx >> 25) & 1;
    features.has_avx = (ecx >> 28) & 1;
    
    /* 检查扩展特性 */
    if (__get_cpuid_max(0, NULL) >= 7) {
        __cpuid_count(7, 0, eax, ebx, ecx, edx);
        
        features.has_avx2 = (ebx >> 5) & 1;
        features.has_avx512f = (ebx >> 16) & 1;
        features.has_gfni = (ecx >> 8) & 1;
        features.has_vaes = (ecx >> 9) & 1;
        features.has_vpclmulqdq = (ecx >> 10) & 1;
    }
#endif

#elif defined(__aarch64__) || defined(_M_ARM64)
    /* ARM64架构 */
    /* 在ARM上检测特性需要使用特定的方法 */
    /* 这里简化处理 */
    features.has_sse2 = 0;
    features.has_aesni = 0;
    features.has_avx = 0;
    features.has_avx2 = 0;
    features.has_avx512f = 0;
    features.has_gfni = 0;
    features.has_vaes = 0;
    features.has_vpclmulqdq = 0;
#endif

    return features;
}

/* 获取最优的SM4实现方式 */
const char* sm4_get_best_implementation(void) {
    SM4_CPU_Features features = sm4_get_cpu_features();
    
    if (features.has_gfni && features.has_avx512f) {
        return "gfni";
    } else if (features.has_vaes && features.has_avx2) {
        return "vaes";
    } else if (features.has_aesni) {
        return "aesni";
    } else {
        return "t_table";
    }
}

/* 当前强制使用的实现 */
static const char* forced_impl = NULL;

/* 强制使用特定的SM4实现 */
int sm4_force_implementation(const char* impl_name) {
    if (!impl_name) {
        forced_impl = NULL;
        return 0;
    }
    
    if (strcmp(impl_name, "basic") == 0 ||
        strcmp(impl_name, "t_table") == 0 ||
        strcmp(impl_name, "aesni") == 0 ||
        strcmp(impl_name, "gfni") == 0) {
        forced_impl = impl_name;
        return 0;
    }
    
    return -1; /* 不支持的实现 */
}