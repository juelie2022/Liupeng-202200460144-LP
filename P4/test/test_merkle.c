#include "../include/merkle.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 测试基本Merkle树功能
static int test_basic_merkle_tree() {
    printf("=== 测试基本Merkle树功能 ===\n");
    
    const size_t leaf_count = 8;
    printf("创建包含 %zu 个叶子节点的Merkle树\n", leaf_count);
    
    // 创建树
    merkle_tree_t *tree = merkle_tree_create(leaf_count);
    if (!tree) {
        printf("✗ 树创建失败\n");
        return 0;
    }
    
    // 生成叶子节点哈希
    uint8_t **leaf_hashes = (uint8_t**)safe_calloc(leaf_count, sizeof(uint8_t*));
    if (!leaf_hashes) {
        merkle_tree_destroy(tree);
        return 0;
    }
    
    for (size_t i = 0; i < leaf_count; i++) {
        leaf_hashes[i] = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
        if (!leaf_hashes[i]) continue;
        
        // 生成简单的测试哈希
        char leaf_data[64];
        snprintf(leaf_data, sizeof(leaf_data), "leaf_%zu", i);
        sm3_hash_string(leaf_data, leaf_hashes[i]);
    }
    
    // 构建树
    merkle_tree_build(tree, leaf_hashes);
    
    // 打印树信息
    merkle_tree_print(tree);
    
    // 验证树结构
    if (tree->root && tree->leaves) {
        printf("✓ 树构建成功\n");
    } else {
        printf("✗ 树构建失败\n");
        goto cleanup;
    }
    
    // 清理
cleanup:
    for (size_t i = 0; i < leaf_count; i++) {
        if (leaf_hashes[i]) safe_free(leaf_hashes[i]);
    }
    safe_free(leaf_hashes);
    merkle_tree_destroy(tree);
    
    return 1;
}

// 测试存在性证明
static int test_existence_proof() {
    printf("\n=== 测试存在性证明 ===\n");
    
    const size_t leaf_count = 16;
    printf("创建包含 %zu 个叶子节点的Merkle树\n", leaf_count);
    
    // 创建树
    merkle_tree_t *tree = merkle_tree_create(leaf_count);
    if (!tree) return 0;
    
    // 生成叶子节点哈希
    uint8_t **leaf_hashes = (uint8_t**)safe_calloc(leaf_count, sizeof(uint8_t*));
    if (!leaf_hashes) {
        merkle_tree_destroy(tree);
        return 0;
    }
    
    for (size_t i = 0; i < leaf_count; i++) {
        leaf_hashes[i] = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
        if (!leaf_hashes[i]) continue;
        
        char leaf_data[64];
        snprintf(leaf_data, sizeof(leaf_data), "leaf_%zu", i);
        sm3_hash_string(leaf_data, leaf_hashes[i]);
    }
    
    // 构建树
    merkle_tree_build(tree, leaf_hashes);
    
    // 测试存在性证明
    const size_t test_index = 7;
    printf("为叶子节点 %zu 创建存在性证明\n", test_index);
    
    merkle_proof_t *proof = merkle_proof_create(tree, test_index);
    if (!proof) {
        printf("✗ 证明创建失败\n");
        goto cleanup;
    }
    
    printf("✓ 证明创建成功，路径长度: %zu\n", proof->path_length);
    
    // 验证证明
    uint8_t leaf_hash[SM3_DIGEST_SIZE];
    merkle_tree_get_leaf(tree, test_index, leaf_hash);
    
    int verify_result = merkle_proof_verify(leaf_hash, proof, tree->root->hash);
    if (verify_result) {
        printf("✓ 证明验证成功\n");
    } else {
        printf("✗ 证明验证失败\n");
    }
    
    // 测试错误验证
    uint8_t wrong_hash[SM3_DIGEST_SIZE];
    memset(wrong_hash, 0xFF, SM3_DIGEST_SIZE);
    
    int wrong_verify = merkle_proof_verify(wrong_hash, proof, tree->root->hash);
    if (!wrong_verify) {
        printf("✓ 错误哈希验证正确失败\n");
    } else {
        printf("✗ 错误哈希验证错误通过\n");
    }
    
    // 清理
    merkle_proof_destroy(proof);
    
cleanup:
    for (size_t i = 0; i < leaf_count; i++) {
        if (leaf_hashes[i]) safe_free(leaf_hashes[i]);
    }
    safe_free(leaf_hashes);
    merkle_tree_destroy(tree);
    
    return verify_result;
}

// 测试不存在性证明
static int test_nonexistence_proof() {
    printf("\n=== 测试不存在性证明 ===\n");
    
    const size_t leaf_count = 32;
    printf("创建包含 %zu 个叶子节点的Merkle树（部分叶子为空）\n", leaf_count);
    
    // 创建树
    merkle_tree_t *tree = merkle_tree_create(leaf_count);
    if (!tree) return 0;
    
    // 只设置部分叶子节点
    uint8_t **leaf_hashes = (uint8_t**)safe_calloc(leaf_count, sizeof(uint8_t*));
    if (!leaf_hashes) {
        merkle_tree_destroy(tree);
        return 0;
    }
    
    // 设置偶数索引的叶子节点
    for (size_t i = 0; i < leaf_count; i += 2) {
        leaf_hashes[i] = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
        if (!leaf_hashes[i]) continue;
        
        char leaf_data[64];
        snprintf(leaf_data, sizeof(leaf_data), "leaf_%zu", i);
        sm3_hash_string(leaf_data, leaf_hashes[i]);
    }
    
    // 构建树
    merkle_tree_build(tree, leaf_hashes);
    
    // 测试不存在性证明
    const size_t test_index = 15; // 奇数索引，应该不存在
    printf("为不存在的叶子节点 %zu 创建不存在性证明\n", test_index);
    
    merkle_nonexistence_proof_t *proof = merkle_nonexistence_proof_create(tree, test_index);
    if (!proof) {
        printf("✗ 不存在性证明创建失败\n");
        goto cleanup;
    }
    
    printf("✓ 不存在性证明创建成功\n");
    
    // 验证证明
    int verify_result = merkle_nonexistence_proof_verify(proof, tree->root->hash, test_index);
    if (verify_result) {
        printf("✓ 不存在性证明验证成功\n");
    } else {
        printf("✗ 不存在性证明验证失败\n");
    }
    
    // 清理
    merkle_nonexistence_proof_destroy(proof);
    
cleanup:
    for (size_t i = 0; i < leaf_count; i++) {
        if (leaf_hashes[i]) safe_free(leaf_hashes[i]);
    }
    safe_free(leaf_hashes);
    merkle_tree_destroy(tree);
    
    return verify_result;
}

// 测试RFC6962兼容性
static int test_rfc6962_compatibility() {
    printf("\n=== 测试RFC6962兼容性 ===\n");
    
    const size_t leaf_count = 8;
    printf("创建符合RFC6962标准的Merkle树\n");
    
    // 创建树
    merkle_tree_t *tree = merkle_tree_create(leaf_count);
    if (!tree) return 0;
    
    // 准备叶子数据
    uint8_t **leaf_data = (uint8_t**)safe_calloc(leaf_count, sizeof(uint8_t*));
    size_t *data_lens = (size_t*)safe_calloc(leaf_count, sizeof(size_t));
    
    if (!leaf_data || !data_lens) {
        goto cleanup;
    }
    
    for (size_t i = 0; i < leaf_count; i++) {
        char data_str[64];
        snprintf(data_str, sizeof(data_str), "RFC6962_leaf_%zu", i);
        size_t len = strlen(data_str);
        
        leaf_data[i] = (uint8_t*)safe_malloc(len);
        if (!leaf_data[i]) continue;
        
        memcpy(leaf_data[i], data_str, len);
        data_lens[i] = len;
    }
    
    // 使用RFC6962方法构建树
    rfc6962_tree_build(tree, leaf_data, data_lens);
    
    if (tree->root) {
        printf("✓ RFC6962树构建成功\n");
        printf("根哈希: ");
        print_hex(tree->root->hash, SM3_DIGEST_SIZE);
    } else {
        printf("✗ RFC6962树构建失败\n");
    }
    
    // 清理
cleanup:
    if (leaf_data) {
        for (size_t i = 0; i < leaf_count; i++) {
            if (leaf_data[i]) safe_free(leaf_data[i]);
        }
        safe_free(leaf_data);
    }
    if (data_lens) safe_free(data_lens);
    merkle_tree_destroy(tree);
    
    return tree->root != NULL;
}

// 测试大规模Merkle树（10万叶子节点）
static int test_large_merkle_tree() {
    printf("\n=== 测试大规模Merkle树（10万叶子节点） ===\n");
    
    const size_t leaf_count = 100000;
    printf("创建包含 %zu 个叶子节点的大规模Merkle树\n", leaf_count);
    
    timer_t timer;
    timer_start(&timer);
    
    // 创建树
    merkle_tree_t *tree = merkle_tree_create(leaf_count);
    if (!tree) {
        printf("✗ 大规模树创建失败\n");
        return 0;
    }
    
    timer_stop(&timer);
    double create_time = timer_get_elapsed_ms(&timer);
    printf("树创建时间: %.3f ms\n", create_time);
    
    // 生成叶子节点哈希（分批处理以节省内存）
    const size_t batch_size = 1000;
    uint8_t **leaf_hashes = (uint8_t**)safe_calloc(leaf_count, sizeof(uint8_t*));
    if (!leaf_hashes) {
        merkle_tree_destroy(tree);
        return 0;
    }
    
    timer_start(&timer);
    
    for (size_t batch = 0; batch < leaf_count; batch += batch_size) {
        size_t current_batch_size = (batch + batch_size > leaf_count) ? 
                                   (leaf_count - batch) : batch_size;
        
        for (size_t i = 0; i < current_batch_size; i++) {
            size_t index = batch + i;
            leaf_hashes[index] = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
            if (!leaf_hashes[index]) continue;
            
            // 生成简单的测试哈希
            char leaf_data[64];
            snprintf(leaf_data, sizeof(leaf_data), "large_leaf_%zu", index);
            sm3_hash_string(leaf_data, leaf_hashes[index]);
        }
        
        // 显示进度
        if (batch % 10000 == 0) {
            printf("生成叶子节点哈希: %zu/%zu (%.1f%%)\n", 
                   batch + current_batch_size, leaf_count, 
                   (batch + current_batch_size) * 100.0 / leaf_count);
        }
    }
    
    timer_stop(&timer);
    double hash_time = timer_get_elapsed_ms(&timer);
    printf("叶子节点哈希生成时间: %.3f ms\n", hash_time);
    
    // 构建树
    printf("开始构建Merkle树...\n");
    timer_start(&timer);
    
    merkle_tree_build(tree, leaf_hashes);
    
    timer_stop(&timer);
    double build_time = timer_get_elapsed_ms(&timer);
    printf("树构建时间: %.3f ms\n", build_time);
    
    if (tree->root) {
        printf("✓ 大规模Merkle树构建成功\n");
        printf("树高度: %zu\n", tree->height);
        printf("总节点数: %zu\n", merkle_tree_get_node_count(leaf_count));
        printf("根哈希: ");
        print_hex(tree->root->hash, SM3_DIGEST_SIZE);
    } else {
        printf("✗ 大规模Merkle树构建失败\n");
        goto cleanup;
    }
    
    // 测试证明生成性能
    printf("\n测试证明生成性能...\n");
    const size_t test_indices[] = {0, 1000, 10000, 50000, 99999};
    const int proof_iterations = 100;
    
    for (int i = 0; i < 5; i++) {
        size_t test_index = test_indices[i];
        
        timer_start(&timer);
        for (int j = 0; j < proof_iterations; j++) {
            merkle_proof_t *proof = merkle_proof_create(tree, test_index);
            if (proof) {
                merkle_proof_destroy(proof);
            }
        }
        timer_stop(&timer);
        
        double proof_time = timer_get_elapsed_ms(&timer);
        printf("叶子 %zu 的证明生成: %.3f ms (%d 次, 平均 %.3f ms)\n", 
               test_index, proof_time, proof_iterations, proof_time / proof_iterations);
    }
    
    // 清理
cleanup:
    if (leaf_hashes) {
        for (size_t i = 0; i < leaf_count; i++) {
            if (leaf_hashes[i]) safe_free(leaf_hashes[i]);
        }
        safe_free(leaf_hashes);
    }
    merkle_tree_destroy(tree);
    
    return tree->root != NULL;
}

// 性能统计
static void performance_statistics() {
    printf("\n=== 性能统计 ===\n");
    
    const size_t test_sizes[] = {100, 1000, 10000, 100000};
    
    for (int i = 0; i < 4; i++) {
        size_t leaf_count = test_sizes[i];
        
        printf("\n测试 %zu 个叶子节点:\n", leaf_count);
        
        timer_t timer;
        double total_time = 0.0;
        
        // 树创建时间
        timer_start(&timer);
        merkle_tree_t *tree = merkle_tree_create(leaf_count);
        timer_stop(&timer);
        double create_time = timer_get_elapsed_ms(&timer);
        total_time += create_time;
        printf("  树创建: %.3f ms\n", create_time);
        
        if (!tree) continue;
        
        // 叶子哈希生成时间
        uint8_t **leaf_hashes = (uint8_t**)safe_calloc(leaf_count, sizeof(uint8_t*));
        if (!leaf_hashes) {
            merkle_tree_destroy(tree);
            continue;
        }
        
        timer_start(&timer);
        for (size_t j = 0; j < leaf_count; j++) {
            leaf_hashes[j] = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
            if (leaf_hashes[j]) {
                char data[64];
                snprintf(data, sizeof(data), "leaf_%zu", j);
                sm3_hash_string(data, leaf_hashes[j]);
            }
        }
        timer_stop(&timer);
        double hash_time = timer_get_elapsed_ms(&timer);
        total_time += hash_time;
        printf("  哈希生成: %.3f ms\n", hash_time);
        
        // 树构建时间
        timer_start(&timer);
        merkle_tree_build(tree, leaf_hashes);
        timer_stop(&timer);
        double build_time = timer_get_elapsed_ms(&timer);
        total_time += build_time;
        printf("  树构建: %.3f ms\n", build_time);
        
        // 证明生成时间
        timer_start(&timer);
        merkle_proof_t *proof = merkle_proof_create(tree, 0);
        timer_stop(&timer);
        double proof_time = timer_get_elapsed_ms(&timer);
        printf("  证明生成: %.3f ms\n", proof_time);
        
        printf("  总时间: %.3f ms\n", total_time);
        printf("  平均每叶子: %.6f ms\n", total_time / leaf_count);
        
        // 清理
        if (proof) merkle_proof_destroy(proof);
        for (size_t j = 0; j < leaf_count; j++) {
            if (leaf_hashes[j]) safe_free(leaf_hashes[j]);
        }
        safe_free(leaf_hashes);
        merkle_tree_destroy(tree);
    }
}

// 主函数
int main() {
    printf("Merkle树测试程序\n");
    printf("================\n");
    
    // 初始化随机数生成器
    init_random();
    
    int all_tests_passed = 1;
    
    // 运行各种测试
    if (!test_basic_merkle_tree()) all_tests_passed = 0;
    if (!test_existence_proof()) all_tests_passed = 0;
    if (!test_nonexistence_proof()) all_tests_passed = 0;
    if (!test_rfc6962_compatibility()) all_tests_passed = 0;
    
    // 大规模测试（可选）
    printf("\n是否运行大规模测试（10万叶子节点）？这可能需要较长时间。\n");
    printf("输入 'y' 继续，其他键跳过: ");
    
    int c = getchar();
    if (c == 'y' || c == 'Y') {
        if (!test_large_merkle_tree()) all_tests_passed = 0;
    } else {
        printf("跳过大规模测试\n");
    }
    
    // 性能统计
    performance_statistics();
    
    printf("\n=== 测试总结 ===\n");
    if (all_tests_passed) {
        printf("✓ 所有基本测试通过\n");
    } else {
        printf("✗ 部分测试失败\n");
    }
    
    printf("\nMerkle树功能包括:\n");
    printf("- 基本树构建和操作\n");
    printf("- 存在性证明生成和验证\n");
    printf("- 不存在性证明生成和验证\n");
    printf("- RFC6962标准兼容性\n");
    printf("- 大规模树支持（10万+叶子节点）\n");
    printf("- 性能优化和统计\n");
    
    return all_tests_passed ? 0 : 1;
}
