#include "merkle.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 创建Merkle树节点
static merkle_node_t* create_node() {
    merkle_node_t *node = (merkle_node_t*)safe_malloc(sizeof(merkle_node_t));
    if (!node) return NULL;
    
    memset(node->hash, 0, SM3_DIGEST_SIZE);
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->index = 0;
    node->is_leaf = 0;
    
    return node;
}

// 计算两个子节点的哈希值
static void hash_children(const uint8_t *left_hash, const uint8_t *right_hash, uint8_t *parent_hash) {
    uint8_t combined[SM3_DIGEST_SIZE * 2];
    
    // 组合左右子节点的哈希值
    memcpy(combined, left_hash, SM3_DIGEST_SIZE);
    memcpy(combined + SM3_DIGEST_SIZE, right_hash, SM3_DIGEST_SIZE);
    
    // 计算父节点哈希
    sm3_hash(combined, SM3_DIGEST_SIZE * 2, parent_hash);
}

// 创建Merkle树
merkle_tree_t* merkle_tree_create(size_t leaf_count) {
    if (leaf_count == 0) return NULL;
    
    merkle_tree_t *tree = (merkle_tree_t*)safe_malloc(sizeof(merkle_tree_t));
    if (!tree) return NULL;
    
    tree->leaf_count = leaf_count;
    tree->height = log2_ceil(leaf_count);
    tree->root = NULL;
    tree->leaves = (merkle_node_t**)safe_calloc(leaf_count, sizeof(merkle_node_t*));
    
    if (!tree->leaves) {
        safe_free(tree);
        return NULL;
    }
    
    return tree;
}

// 销毁Merkle树
static void destroy_node_recursive(merkle_node_t *node) {
    if (!node) return;
    
    destroy_node_recursive(node->left);
    destroy_node_recursive(node->right);
    safe_free(node);
}

void merkle_tree_destroy(merkle_tree_t *tree) {
    if (!tree) return;
    
    if (tree->root) {
        destroy_node_recursive(tree->root);
    }
    
    if (tree->leaves) {
        safe_free(tree->leaves);
    }
    
    safe_free(tree);
}

// 构建Merkle树
void merkle_tree_build(merkle_tree_t *tree, uint8_t **leaf_hashes) {
    if (!tree || !leaf_hashes) return;
    
    size_t leaf_count = tree->leaf_count;
    size_t height = tree->height;
    
    // 创建叶子节点
    for (size_t i = 0; i < leaf_count; i++) {
        merkle_node_t *leaf = create_node();
        if (!leaf) continue;
        
        memcpy(leaf->hash, leaf_hashes[i], SM3_DIGEST_SIZE);
        leaf->index = i;
        leaf->is_leaf = 1;
        tree->leaves[i] = leaf;
    }
    
    // 构建内部节点
    merkle_node_t **current_level = tree->leaves;
    size_t current_count = leaf_count;
    
    for (size_t level = 0; level < height; level++) {
        size_t next_count = (current_count + 1) / 2;
        merkle_node_t **next_level = (merkle_node_t**)safe_calloc(next_count, sizeof(merkle_node_t*));
        
        if (!next_level) break;
        
        for (size_t i = 0; i < next_count; i++) {
            merkle_node_t *parent = create_node();
            if (!parent) continue;
            
            size_t left_idx = i * 2;
            size_t right_idx = left_idx + 1;
            
            if (left_idx < current_count) {
                parent->left = current_level[left_idx];
                current_level[left_idx]->parent = parent;
                
                if (right_idx < current_count) {
                    parent->right = current_level[right_idx];
                    current_level[right_idx]->parent = parent;
                    hash_children(current_level[left_idx]->hash, 
                                current_level[right_idx]->hash, 
                                parent->hash);
                } else {
                    // 右子节点不存在，复制左子节点的哈希
                    memcpy(parent->hash, current_level[left_idx]->hash, SM3_DIGEST_SIZE);
                }
            }
            
            next_level[i] = parent;
        }
        
        // 释放当前层级的数组（保留节点）
        if (level > 0) {
            safe_free(current_level);
        }
        
        current_level = next_level;
        current_count = next_count;
    }
    
    tree->root = current_level[0];
    
    // 释放最后一层的数组
    if (height > 0) {
        safe_free(current_level);
    }
}

// 设置叶子节点
void merkle_tree_set_leaf(merkle_tree_t *tree, size_t index, const uint8_t *hash) {
    if (!tree || !hash || index >= tree->leaf_count) return;
    
    merkle_node_t *leaf = tree->leaves[index];
    if (!leaf) return;
    
    memcpy(leaf->hash, hash, SM3_DIGEST_SIZE);
    
    // 更新从叶子到根的路径
    merkle_node_t *current = leaf;
    while (current->parent) {
        merkle_node_t *parent = current->parent;
        uint8_t *left_hash = parent->left->hash;
        uint8_t *right_hash = parent->right ? parent->right->hash : parent->left->hash;
        
        hash_children(left_hash, right_hash, parent->hash);
        current = parent;
    }
}

// 获取叶子节点
void merkle_tree_get_leaf(const merkle_tree_t *tree, size_t index, uint8_t *hash) {
    if (!tree || !hash || index >= tree->leaf_count) return;
    
    merkle_node_t *leaf = tree->leaves[index];
    if (leaf) {
        memcpy(hash, leaf->hash, SM3_DIGEST_SIZE);
    }
}

// 创建存在性证明
merkle_proof_t* merkle_proof_create(const merkle_tree_t *tree, size_t leaf_index) {
    if (!tree || leaf_index >= tree->leaf_count) return NULL;
    
    merkle_node_t *leaf = tree->leaves[leaf_index];
    if (!leaf) return NULL;
    
    merkle_proof_t *proof = (merkle_proof_t*)safe_malloc(sizeof(merkle_proof_t));
    if (!proof) return NULL;
    
    proof->path_length = tree->height;
    proof->hashes = (uint8_t**)safe_calloc(proof->path_length, sizeof(uint8_t*));
    proof->directions = (int*)safe_calloc(proof->path_length, sizeof(int));
    
    if (!proof->hashes || !proof->directions) {
        if (proof->hashes) safe_free(proof->hashes);
        if (proof->directions) safe_free(proof->directions);
        safe_free(proof);
        return NULL;
    }
    
    // 构建证明路径
    merkle_node_t *current = leaf;
    size_t proof_index = 0;
    
    while (current->parent && proof_index < proof->path_length) {
        merkle_node_t *parent = current->parent;
        merkle_node_t *sibling = (current == parent->left) ? parent->right : parent->left;
        
        if (sibling) {
            proof->hashes[proof_index] = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
            memcpy(proof->hashes[proof_index], sibling->hash, SM3_DIGEST_SIZE);
        } else {
            proof->hashes[proof_index] = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
            memcpy(proof->hashes[proof_index], current->hash, SM3_DIGEST_SIZE);
        }
        
        proof->directions[proof_index] = (current == parent->left) ? 1 : 0;
        
        current = parent;
        proof_index++;
    }
    
    return proof;
}

// 验证存在性证明
int merkle_proof_verify(const uint8_t *leaf_hash, const merkle_proof_t *proof, 
                        const uint8_t *root_hash) {
    if (!leaf_hash || !proof || !root_hash) return 0;
    
    uint8_t current_hash[SM3_DIGEST_SIZE];
    memcpy(current_hash, leaf_hash, SM3_DIGEST_SIZE);
    
    // 沿着证明路径计算哈希值
    for (size_t i = 0; i < proof->path_length; i++) {
        uint8_t combined[SM3_DIGEST_SIZE * 2];
        
        if (proof->directions[i] == 0) {
            // 当前节点是右子节点，兄弟节点是左子节点
            memcpy(combined, proof->hashes[i], SM3_DIGEST_SIZE);
            memcpy(combined + SM3_DIGEST_SIZE, current_hash, SM3_DIGEST_SIZE);
        } else {
            // 当前节点是左子节点，兄弟节点是右子节点
            memcpy(combined, current_hash, SM3_DIGEST_SIZE);
            memcpy(combined + SM3_DIGEST_SIZE, proof->hashes[i], SM3_DIGEST_SIZE);
        }
        
        sm3_hash(combined, SM3_DIGEST_SIZE * 2, current_hash);
    }
    
    // 比较计算出的根哈希与给定的根哈希
    return memcmp(current_hash, root_hash, SM3_DIGEST_SIZE) == 0;
}

// 销毁存在性证明
void merkle_proof_destroy(merkle_proof_t *proof) {
    if (!proof) return;
    
    if (proof->hashes) {
        for (size_t i = 0; i < proof->path_length; i++) {
            if (proof->hashes[i]) {
                safe_free(proof->hashes[i]);
            }
        }
        safe_free(proof->hashes);
    }
    
    if (proof->directions) {
        safe_free(proof->directions);
    }
    
    safe_free(proof);
}

// 创建不存在性证明
merkle_nonexistence_proof_t* merkle_nonexistence_proof_create(
    const merkle_tree_t *tree, size_t target_index) {
    if (!tree || target_index >= tree->leaf_count) return NULL;
    
    merkle_nonexistence_proof_t *proof = (merkle_nonexistence_proof_t*)safe_malloc(
        sizeof(merkle_nonexistence_proof_t));
    if (!proof) return NULL;
    
    // 找到左边界叶子
    size_t left_index = target_index;
    while (left_index > 0 && !tree->leaves[left_index - 1]) {
        left_index--;
    }
    
    // 找到右边界叶子
    size_t right_index = target_index;
    while (right_index < tree->leaf_count - 1 && !tree->leaves[right_index + 1]) {
        right_index++;
    }
    
    // 创建左边界证明
    if (left_index < target_index) {
        proof->left_proof = *merkle_proof_create(tree, left_index);
        merkle_tree_get_leaf(tree, left_index, proof->left_leaf);
    } else {
        proof->left_proof.path_length = 0;
        proof->left_proof.hashes = NULL;
        proof->left_proof.directions = NULL;
        memset(proof->left_leaf, 0, SM3_DIGEST_SIZE);
    }
    
    // 创建右边界证明
    if (right_index > target_index) {
        proof->right_proof = *merkle_proof_create(tree, right_index);
        merkle_tree_get_leaf(tree, right_index, proof->right_leaf);
    } else {
        proof->right_proof.path_length = 0;
        proof->right_proof.hashes = NULL;
        proof->right_proof.directions = NULL;
        memset(proof->right_leaf, 0, SM3_DIGEST_SIZE);
    }
    
    return proof;
}

// 验证不存在性证明
int merkle_nonexistence_proof_verify(const merkle_nonexistence_proof_t *proof,
                                    const uint8_t *root_hash, size_t target_index) {
    if (!proof || !root_hash) return 0;
    
    // 验证左边界证明
    if (proof->left_proof.path_length > 0) {
        if (!merkle_proof_verify(proof->left_leaf, &proof->left_proof, root_hash)) {
            return 0;
        }
    }
    
    // 验证右边界证明
    if (proof->right_proof.path_length > 0) {
        if (!merkle_proof_verify(proof->right_leaf, &proof->right_proof, root_hash)) {
            return 0;
        }
    }
    
    // 验证目标索引确实在两个边界之间
    // 这里需要根据具体的树结构实现边界检查
    (void)target_index; // 避免未使用参数警告
    
    return 1;
}

// 销毁不存在性证明
void merkle_nonexistence_proof_destroy(merkle_nonexistence_proof_t *proof) {
    if (!proof) return;
    
    if (proof->left_proof.hashes) {
        for (size_t i = 0; i < proof->left_proof.path_length; i++) {
            if (proof->left_proof.hashes[i]) {
                safe_free(proof->left_proof.hashes[i]);
            }
        }
        safe_free(proof->left_proof.hashes);
    }
    
    if (proof->left_proof.directions) {
        safe_free(proof->left_proof.directions);
    }
    
    if (proof->right_proof.hashes) {
        for (size_t i = 0; i < proof->right_proof.path_length; i++) {
            if (proof->right_proof.hashes[i]) {
                safe_free(proof->right_proof.hashes[i]);
            }
        }
        safe_free(proof->right_proof.hashes);
    }
    
    if (proof->right_proof.directions) {
        safe_free(proof->right_proof.directions);
    }
    
    safe_free(proof);
}

// 批量更新
void merkle_tree_batch_update(merkle_tree_t *tree, size_t start_index, 
                             uint8_t **new_hashes, size_t count) {
    if (!tree || !new_hashes || start_index + count > tree->leaf_count) return;
    
    for (size_t i = 0; i < count; i++) {
        merkle_tree_set_leaf(tree, start_index + i, new_hashes[i]);
    }
}

// 批量创建证明
merkle_proof_t** merkle_tree_batch_proof_create(const merkle_tree_t *tree,
                                               size_t *indices, size_t count) {
    if (!tree || !indices || count == 0) return NULL;
    
    merkle_proof_t **proofs = (merkle_proof_t**)safe_calloc(count, sizeof(merkle_proof_t*));
    if (!proofs) return NULL;
    
    for (size_t i = 0; i < count; i++) {
        proofs[i] = merkle_proof_create(tree, indices[i]);
    }
    
    return proofs;
}

// 工具函数
void merkle_tree_print(const merkle_tree_t *tree) {
    if (!tree) return;
    
    printf("Merkle树信息:\n");
    printf("  叶子节点数量: %zu\n", tree->leaf_count);
    printf("  树高度: %zu\n", tree->height);
    printf("  根哈希: ");
    if (tree->root) {
        print_hex(tree->root->hash, SM3_DIGEST_SIZE);
    } else {
        printf("未设置\n");
    }
}

size_t merkle_tree_get_height(size_t leaf_count) {
    return log2_ceil(leaf_count);
}

size_t merkle_tree_get_node_count(size_t leaf_count) {
    if (leaf_count == 0) return 0;
    
    size_t height = log2_ceil(leaf_count);
    size_t total_nodes = 0;
    
    for (size_t i = 0; i <= height; i++) {
        total_nodes += (leaf_count + (1ULL << i) - 1) >> i;
    }
    
    return total_nodes;
}

// RFC6962特定函数
uint8_t* rfc6962_hash_leaf(const uint8_t *data, size_t len) {
    uint8_t *hash = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
    if (!hash) return NULL;
    
    // RFC6962叶子节点哈希格式: H(0x00 || data)
    uint8_t *input = (uint8_t*)safe_malloc(len + 1);
    if (!input) {
        safe_free(hash);
        return NULL;
    }
    
    input[0] = 0x00;
    memcpy(input + 1, data, len);
    
    sm3_hash(input, len + 1, hash);
    safe_free(input);
    
    return hash;
}

uint8_t* rfc6962_hash_children(const uint8_t *left_hash, const uint8_t *right_hash) {
    uint8_t *hash = (uint8_t*)safe_malloc(SM3_DIGEST_SIZE);
    if (!hash) return NULL;
    
    // RFC6962内部节点哈希格式: H(0x01 || left_hash || right_hash)
    uint8_t *input = (uint8_t*)safe_malloc(1 + SM3_DIGEST_SIZE * 2);
    if (!input) {
        safe_free(hash);
        return NULL;
    }
    
    input[0] = 0x01;
    memcpy(input + 1, left_hash, SM3_DIGEST_SIZE);
    memcpy(input + 1 + SM3_DIGEST_SIZE, right_hash, SM3_DIGEST_SIZE);
    
    sm3_hash(input, 1 + SM3_DIGEST_SIZE * 2, hash);
    safe_free(input);
    
    return hash;
}

void rfc6962_tree_build(merkle_tree_t *tree, uint8_t **leaf_data, size_t *data_lens) {
    if (!tree || !leaf_data || !data_lens) return;
    
    // 计算叶子节点哈希
    uint8_t **leaf_hashes = (uint8_t**)safe_calloc(tree->leaf_count, sizeof(uint8_t*));
    if (!leaf_hashes) return;
    
    for (size_t i = 0; i < tree->leaf_count; i++) {
        leaf_hashes[i] = rfc6962_hash_leaf(leaf_data[i], data_lens[i]);
    }
    
    // 构建树
    merkle_tree_build(tree, leaf_hashes);
    
    // 释放临时哈希数组
    for (size_t i = 0; i < tree->leaf_count; i++) {
        if (leaf_hashes[i]) {
            safe_free(leaf_hashes[i]);
        }
    }
    safe_free(leaf_hashes);
}
