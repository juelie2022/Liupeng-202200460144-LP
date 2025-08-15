#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 内存操作
void* safe_malloc(size_t size);
void* safe_calloc(size_t count, size_t size);
void* safe_realloc(void *ptr, size_t size);
void safe_free(void *ptr);

// 字节序转换
uint32_t swap_endian_32(uint32_t value);
uint64_t swap_endian_64(uint64_t value);
void swap_endian_array_32(uint32_t *array, size_t count);
void swap_endian_array_64(uint64_t *array, size_t count);

// 十六进制转换
void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex);
int hex_to_bytes(const char *hex, uint8_t *bytes, size_t max_len);
void print_hex(const uint8_t *data, size_t len);

// 时间测量
typedef struct {
    long long start_time;
    long long end_time;
} timer_t;

void timer_start(timer_t *timer);
void timer_stop(timer_t *timer);
double timer_get_elapsed_ms(timer_t *timer);
double timer_get_elapsed_us(timer_t *timer);

// 随机数生成
void init_random();
uint32_t random_uint32();
uint64_t random_uint64();
void random_bytes(uint8_t *buffer, size_t len);

// 字符串操作
char* safe_strdup(const char *str);
int safe_strcmp(const char *str1, const char *str2);
size_t safe_strlen(const char *str);

// 数学工具
size_t next_power_of_2(size_t n);
size_t log2_ceil(size_t n);
int is_power_of_2(size_t n);

// 错误处理
typedef enum {
    UTILS_SUCCESS = 0,
    UTILS_ERROR_NULL_POINTER,
    UTILS_ERROR_INVALID_PARAMETER,
    UTILS_ERROR_MEMORY_ALLOCATION,
    UTILS_ERROR_BUFFER_OVERFLOW,
    UTILS_ERROR_INVALID_FORMAT
} utils_error_t;

const char* utils_error_string(utils_error_t error);
void utils_set_error_callback(void (*callback)(const char *message));

// 调试工具
#ifdef DEBUG
    #define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
    #define DEBUG_ASSERT(cond) assert(cond)
#else
    #define DEBUG_PRINT(fmt, ...) ((void)0)
    #define DEBUG_ASSERT(cond) ((void)0)
#endif

// 性能统计
typedef struct {
    size_t operation_count;
    double total_time_ms;
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
} performance_stats_t;

void performance_stats_init(performance_stats_t *stats);
void performance_stats_add_sample(performance_stats_t *stats, double time_ms);
void performance_stats_print(const performance_stats_t *stats, const char *operation_name);

#ifdef __cplusplus
}
#endif

#endif // UTILS_H
