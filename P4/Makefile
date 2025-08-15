# SM3项目Makefile
# 支持多种优化级别和编译选项

# 编译器设置
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
DEBUG_CFLAGS = -std=c99 -Wall -Wextra -g -O0
OPTIMIZED_CFLAGS = -std=c99 -Wall -Wextra -O3 -march=native -mtune=native

# 目录设置
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = test
BUILD_DIR = build

# 源文件
SM3_SOURCES = $(SRC_DIR)/sm3.c $(SRC_DIR)/sm3_optimized.c
MERKLE_SOURCES = $(SRC_DIR)/merkle.c
UTILS_SOURCES = $(SRC_DIR)/utils.c

# 目标文件
SM3_OBJECTS = $(SM3_SOURCES:.c=.o)
MERKLE_OBJECTS = $(MERKLE_SOURCES:.c=.o)
UTILS_OBJECTS = $(UTILS_SOURCES:.c=.o)

# 测试可执行文件
SM3_TEST = $(TEST_DIR)/sm3_test
LENGTH_EXTENSION_TEST = $(TEST_DIR)/length_extension_test
MERKLE_TEST = $(TEST_DIR)/merkle_test

# 默认目标
all: $(SM3_TEST) $(LENGTH_EXTENSION_TEST) $(MERKLE_TEST)

# 调试版本
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: all

# 优化版本
optimized: CFLAGS = $(OPTIMIZED_CFLAGS)
optimized: all

# SM3测试
$(SM3_TEST): $(TEST_DIR)/test_sm3.c $(SM3_OBJECTS) $(UTILS_OBJECTS)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $^ -o $@

# 长度扩展攻击测试
$(LENGTH_EXTENSION_TEST): $(TEST_DIR)/test_length_extension.c $(SM3_OBJECTS) $(UTILS_OBJECTS)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $^ -o $@

# Merkle树测试
$(MERKLE_TEST): $(TEST_DIR)/test_merkle.c $(MERKLE_OBJECTS) $(UTILS_OBJECTS)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $^ -o $@

# 编译源文件
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# 性能测试
benchmark: $(SM3_TEST)
	@echo "运行SM3性能测试..."
	@./$(SM3_TEST) --benchmark

# 清理
clean:
	rm -f $(SRC_DIR)/*.o $(TEST_DIR)/*_test
	rm -rf $(BUILD_DIR)

# 安装依赖（如果需要）
deps:
	@echo "检查依赖..."
	@which $(CC) > /dev/null || (echo "错误: 未找到编译器 $(CC)" && exit 1)
	@echo "依赖检查完成"

# 帮助信息
help:
	@echo "可用的make目标:"
	@echo "  all          - 编译所有目标（默认）"
	@echo "  debug        - 编译调试版本"
	@echo "  optimized    - 编译优化版本"
	@echo "  benchmark    - 运行性能测试"
	@echo "  clean        - 清理编译文件"
	@echo "  deps         - 检查依赖"
	@echo "  help         - 显示此帮助信息"

.PHONY: all debug optimized benchmark clean deps help
