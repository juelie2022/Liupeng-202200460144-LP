#ifndef MERKLE_H
#define MERKLE_H

#include <stdint.h>
#include <stddef.h>
#include "sm3.h"

#ifdef __cplusplus
extern "C" {
#endif

// Merkle树节点结构
typedef struct merkle_node {
    uint8_t hash[SM3_DIGEST_SIZE];  // 节点哈希值
    struct merkle_node *left;        // 左子节点
    struct merkle_node *right;       // 右子节点
    struct merkle_node *parent;      // 父节点
    size_t index;                    // 节点索引
    int is_leaf;                     // 是否为叶子节点
} merkle_node_t;

// Merkle树结构
typedef struct {
    merkle_node_t *root;             // 根节点
    size_t leaf_count;               // 叶子节点数量
    size_t height;                   // 树高度
    merkle_node_t **leaves;          // 叶子节点数组
} merkle_tree_t;

// 证明路径结构
typedef struct {
    size_t path_length;              // 路径长度
    uint8_t **hashes;                // 路径上的哈希值
    int *directions;                 // 方向数组 (0=左, 1=右)
} merkle_proof_t;

// 不存在性证明结构
typedef struct {
    merkle_proof_t left_proof;       // 左边界证明
    merkle_proof_t right_proof;      // 右边界证明
    uint8_t left_leaf[SM3_DIGEST_SIZE];  // 左边界叶子
    uint8_t right_leaf[SM3_DIGEST_SIZE]; // 右边界叶子
} merkle_nonexistence_proof_t;

// 基本Merkle树操作
merkle_tree_t* merkle_tree_create(size_t leaf_count);
void merkle_tree_destroy(merkle_tree_t *tree);
void merkle_tree_build(merkle_tree_t *tree, uint8_t **leaf_hashes);

// 叶子节点操作
void merkle_tree_set_leaf(merkle_tree_t *tree, size_t index, const uint8_t *hash);
void merkle_tree_get_leaf(const merkle_tree_t *tree, size_t index, uint8_t *hash);

// 存在性证明
merkle_proof_t* merkle_proof_create(const merkle_tree_t *tree, size_t leaf_index);
int merkle_proof_verify(const uint8_t *leaf_hash, const merkle_proof_t *proof, 
                        const uint8_t *root_hash);
void merkle_proof_destroy(merkle_proof_t *proof);

// 不存在性证明
merkle_nonexistence_proof_t* merkle_nonexistence_proof_create(
    const merkle_tree_t *tree, size_t target_index);
int merkle_nonexistence_proof_verify(const merkle_nonexistence_proof_t *proof,
                                    const uint8_t *root_hash, size_t target_index);
void merkle_nonexistence_proof_destroy(merkle_nonexistence_proof_t *proof);

// 批量操作
void merkle_tree_batch_update(merkle_tree_t *tree, size_t start_index, 
                             uint8_t **new_hashes, size_t count);
merkle_proof_t** merkle_tree_batch_proof_create(const merkle_tree_t *tree,
                                               size_t *indices, size_t count);

// 工具函数
void merkle_tree_print(const merkle_tree_t *tree);
size_t merkle_tree_get_height(size_t leaf_count);
size_t merkle_tree_get_node_count(size_t leaf_count);

// RFC6962特定函数
uint8_t* rfc6962_hash_leaf(const uint8_t *data, size_t len);
uint8_t* rfc6962_hash_children(const uint8_t *left_hash, const uint8_t *right_hash);
void rfc6962_tree_build(merkle_tree_t *tree, uint8_t **leaf_data, size_t *data_lens);

#ifdef __cplusplus
}
#endif

#endif // MERKLE_H
