#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>

// 错误回调函数
static void (*error_callback)(const char *message) = NULL;

// 内存操作
void* safe_malloc(size_t size) {
    if (size == 0) return NULL;
    
    void *ptr = malloc(size);
    if (!ptr) {
        if (error_callback) {
            error_callback("内存分配失败");
        }
        fprintf(stderr, "错误: 内存分配失败 (大小: %zu)\n", size);
        exit(1);
    }
    return ptr;
}

void* safe_calloc(size_t count, size_t size) {
    if (count == 0 || size == 0) return NULL;
    
    void *ptr = calloc(count, size);
    if (!ptr) {
        if (error_callback) {
            error_callback("内存分配失败");
        }
        fprintf(stderr, "错误: 内存分配失败 (数量: %zu, 大小: %zu)\n", count, size);
        exit(1);
    }
    return ptr;
}

void* safe_realloc(void *ptr, size_t size) {
    if (size == 0) {
        safe_free(ptr);
        return NULL;
    }
    
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        if (error_callback) {
            error_callback("内存重新分配失败");
        }
        fprintf(stderr, "错误: 内存重新分配失败 (大小: %zu)\n", size);
        exit(1);
    }
    return new_ptr;
}

void safe_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

// 字节序转换
uint32_t swap_endian_32(uint32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x000000FF) << 24);
}

uint64_t swap_endian_64(uint64_t value) {
    return ((value & 0xFF00000000000000ULL) >> 56) |
           ((value & 0x00FF000000000000ULL) >> 40) |
           ((value & 0x0000FF0000000000ULL) >> 24) |
           ((value & 0x000000FF00000000ULL) >> 8) |
           ((value & 0x00000000FF000000ULL) << 8) |
           ((value & 0x0000000000FF0000ULL) << 24) |
           ((value & 0x000000000000FF00ULL) << 40) |
           ((value & 0x00000000000000FFULL) << 56);
}

void swap_endian_array_32(uint32_t *array, size_t count) {
    for (size_t i = 0; i < count; i++) {
        array[i] = swap_endian_32(array[i]);
    }
}

void swap_endian_array_64(uint64_t *array, size_t count) {
    for (size_t i = 0; i < count; i++) {
        array[i] = swap_endian_64(array[i]);
    }
}

// 十六进制转换
void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex) {
    if (!bytes || !hex) return;
    
    for (size_t i = 0; i < len; i++) {
        sprintf(hex + i * 2, "%02x", bytes[i]);
    }
    hex[len * 2] = '\0';
}

int hex_to_bytes(const char *hex, uint8_t *bytes, size_t max_len) {
    if (!hex || !bytes) return 0;
    
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0 || hex_len / 2 > max_len) return 0;
    
    for (size_t i = 0; i < hex_len; i += 2) {
        char hex_byte[3] = {hex[i], hex[i + 1], '\0'};
        unsigned int byte_val;
        if (sscanf(hex_byte, "%x", &byte_val) != 1) return 0;
        bytes[i / 2] = (uint8_t)byte_val;
    }
    
    return 1;
}

void print_hex(const uint8_t *data, size_t len) {
    if (!data) return;
    
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
        if ((i + 1) % 16 == 0) printf(" ");
        if ((i + 1) % 32 == 0) printf("\n");
    }
    if (len % 32 != 0) printf("\n");
}

// 时间测量
void timer_start(timer_t *timer) {
    if (!timer) return;
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timer->start_time = (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

void timer_stop(timer_t *timer) {
    if (!timer) return;
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timer->end_time = (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

double timer_get_elapsed_ms(timer_t *timer) {
    if (!timer) return 0.0;
    return (timer->end_time - timer->start_time) / 1000.0;
}

double timer_get_elapsed_us(timer_t *timer) {
    if (!timer) return 0.0;
    return (double)(timer->end_time - timer->start_time);
}

// 随机数生成
static int random_initialized = 0;

void init_random() {
    if (!random_initialized) {
        srand((unsigned int)time(NULL));
        random_initialized = 1;
    }
}

uint32_t random_uint32() {
    if (!random_initialized) init_random();
    return (uint32_t)rand() ^ ((uint32_t)rand() << 16);
}

uint64_t random_uint64() {
    if (!random_initialized) init_random();
    return (uint64_t)random_uint32() ^ ((uint64_t)random_uint32() << 32);
}

void random_bytes(uint8_t *buffer, size_t len) {
    if (!buffer || len == 0) return;
    
    if (!random_initialized) init_random();
    
    for (size_t i = 0; i < len; i++) {
        buffer[i] = (uint8_t)(rand() % 256);
    }
}

// 字符串操作
char* safe_strdup(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str) + 1;
    char *dup = (char*)safe_malloc(len);
    memcpy(dup, str, len);
    return dup;
}

int safe_strcmp(const char *str1, const char *str2) {
    if (!str1 && !str2) return 0;
    if (!str1) return -1;
    if (!str2) return 1;
    return strcmp(str1, str2);
}

size_t safe_strlen(const char *str) {
    return str ? strlen(str) : 0;
}

// 数学工具
size_t next_power_of_2(size_t n) {
    if (n == 0) return 1;
    
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

size_t log2_ceil(size_t n) {
    if (n <= 1) return 0;
    
    size_t log = 0;
    while (n > 1) {
        n >>= 1;
        log++;
    }
    return log;
}

int is_power_of_2(size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// 错误处理
const char* utils_error_string(utils_error_t error) {
    switch (error) {
        case UTILS_SUCCESS:
            return "成功";
        case UTILS_ERROR_NULL_POINTER:
            return "空指针错误";
        case UTILS_ERROR_INVALID_PARAMETER:
            return "无效参数错误";
        case UTILS_ERROR_MEMORY_ALLOCATION:
            return "内存分配错误";
        case UTILS_ERROR_BUFFER_OVERFLOW:
            return "缓冲区溢出错误";
        case UTILS_ERROR_INVALID_FORMAT:
            return "格式错误";
        default:
            return "未知错误";
    }
}

void utils_set_error_callback(void (*callback)(const char *message)) {
    error_callback = callback;
}

// 性能统计
void performance_stats_init(performance_stats_t *stats) {
    if (!stats) return;
    
    stats->operation_count = 0;
    stats->total_time_ms = 0.0;
    stats->min_time_ms = 0.0;
    stats->max_time_ms = 0.0;
    stats->avg_time_ms = 0.0;
}

void performance_stats_add_sample(performance_stats_t *stats, double time_ms) {
    if (!stats) return;
    
    if (stats->operation_count == 0) {
        stats->min_time_ms = time_ms;
        stats->max_time_ms = time_ms;
    } else {
        if (time_ms < stats->min_time_ms) stats->min_time_ms = time_ms;
        if (time_ms > stats->max_time_ms) stats->max_time_ms = time_ms;
    }
    
    stats->total_time_ms += time_ms;
    stats->operation_count++;
    stats->avg_time_ms = stats->total_time_ms / stats->operation_count;
}

void performance_stats_print(const performance_stats_t *stats, const char *operation_name) {
    if (!stats || !operation_name) return;
    
    printf("性能统计 - %s:\n", operation_name);
    printf("  操作次数: %zu\n", stats->operation_count);
    printf("  总时间: %.3f ms\n", stats->total_time_ms);
    printf("  平均时间: %.3f ms\n", stats->avg_time_ms);
    printf("  最小时间: %.3f ms\n", stats->min_time_ms);
    printf("  最大时间: %.3f ms\n", stats->max_time_ms);
    printf("  吞吐量: %.2f 操作/秒\n", 
           1000.0 / stats->avg_time_ms);
}
