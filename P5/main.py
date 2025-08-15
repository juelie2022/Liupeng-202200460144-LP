#!/usr/bin/env python3
"""
SM2软件实现优化主程序
演示所有功能包括基础实现、优化版本、误用验证和伪造签名
"""

import time
import sys
from pathlib import Path

# 添加项目路径
sys.path.insert(0, str(Path(__file__).parent))

from sm2.core import SM2
from sm2.optimized import SM2Optimized, SM2Parallel, SM2MemoryOptimized
from poc.signature_misuse import SignatureMisusePOC
from poc.satoshi_forgery import SatoshiForgeryPOC

def demo_basic_sm2():
    """演示基础SM2功能"""
    print("=" * 60)
    print("基础SM2功能演示")
    print("=" * 60)
    
    sm2 = SM2()
    
    # 生成密钥对
    print("1. 生成SM2密钥对...")
    start_time = time.time()
    public_key, private_key = sm2.generate_keypair()
    keygen_time = time.time() - start_time
    
    print(f"   公钥: {public_key}")
    print(f"   私钥: {hex(private_key)}")
    print(f"   密钥生成时间: {keygen_time:.6f}秒")
    
    # 签名和验证
    print("\n2. 数字签名演示...")
    message = b"Hello, SM2! This is a test message for digital signature."
    
    start_time = time.time()
    signature = sm2.sign(message, private_key)
    sign_time = time.time() - start_time
    
    print(f"   原始消息: {message.decode()}")
    print(f"   签名值: r={hex(signature[0])}, s={hex(signature[1])}")
    print(f"   签名时间: {sign_time:.6f}秒")
    
    # 验证签名
    start_time = time.time()
    is_valid = sm2.verify(message, signature, public_key)
    verify_time = time.time() - start_time
    
    print(f"   签名验证: {'成功' if is_valid else '失败'}")
    print(f"   验证时间: {verify_time:.6f}秒")
    
    # 加密解密
    print("\n3. 加密解密演示...")
    secret_message = b"This is a secret message that will be encrypted using SM2."
    
    start_time = time.time()
    ciphertext = sm2.encrypt(secret_message, public_key)
    encrypt_time = time.time() - start_time
    
    print(f"   原始消息: {secret_message.decode()}")
    print(f"   密文长度: {len(ciphertext)} 字节")
    print(f"   加密时间: {encrypt_time:.6f}秒")
    
    # 解密
    start_time = time.time()
    decrypted_message = sm2.decrypt(ciphertext, private_key)
    decrypt_time = time.time() - start_time
    
    print(f"   解密消息: {decrypted_message.decode()}")
    print(f"   解密时间: {decrypt_time:.6f}秒")
    print(f"   解密成功: {'是' if decrypted_message == secret_message else '否'}")

def demo_optimized_sm2():
    """演示优化版本SM2"""
    print("\n" + "=" * 60)
    print("SM2优化版本演示")
    print("=" * 60)
    
    # 不同优化级别的性能对比
    optimization_levels = ["balanced", "fast"]
    
    for level in optimization_levels:
        print(f"\n{level.upper()} 优化级别:")
        
        sm2_opt = SM2Optimized(optimization_level=level)
        
        # 生成密钥对
        start_time = time.time()
        public_key, private_key = sm2_opt.generate_keypair()
        keygen_time = time.time() - start_time
        
        print(f"   密钥生成时间: {keygen_time:.6f}秒")
        
        # 签名性能
        message = b"Performance test message for optimization comparison."
        
        start_time = time.time()
        signature = sm2_opt.sign_optimized(message, private_key)
        sign_time = time.time() - start_time
        
        print(f"   签名时间: {sign_time:.6f}秒")
        
        # 验证性能
        start_time = time.time()
        is_valid = sm2_opt.verify_optimized(message, signature, public_key)
        verify_time = time.time() - start_time
        
        print(f"   验证时间: {verify_time:.6f}秒")
        print(f"   验证结果: {'成功' if is_valid else '失败'}")
        
        # 获取性能统计
        stats = sm2_opt.get_performance_stats()
        print(f"   预计算表大小: {stats['precompute_table_size']}")
    
    # 并行处理演示
    print("\n并行处理版本:")
    sm2_parallel = SM2Parallel(num_threads=4)
    
    # 批量签名
    messages = [f"Message {i}".encode() for i in range(10)]
    public_key, private_key = sm2_parallel.generate_keypair()
    
    start_time = time.time()
    signatures = sm2_parallel.parallel_sign(messages, private_key)
    parallel_sign_time = time.time() - start_time
    
    print(f"   批量签名时间 (10条消息): {parallel_sign_time:.6f}秒")
    
    # 批量验证
    start_time = time.time()
    results = sm2_parallel.parallel_verify(messages, signatures, [public_key] * 10)
    parallel_verify_time = time.time() - start_time
    
    print(f"   批量验证时间 (10条消息): {parallel_verify_time:.6f}秒")
    print(f"   验证成功率: {sum(results)}/{len(results)}")
    
    # 内存优化版本
    print("\n内存优化版本:")
    sm2_memory = SM2MemoryOptimized()
    
    # 重复操作测试缓存效果
    message = b"Cache test message"
    public_key, private_key = sm2_memory.generate_keypair()
    
    # 第一次签名
    start_time = time.time()
    signature1 = sm2_memory.sign_memory_optimized(message, private_key)
    first_sign_time = time.time() - start_time
    
    # 第二次签名 (应该使用缓存)
    start_time = time.time()
    signature2 = sm2_memory.sign_memory_optimized(message, private_key)
    second_sign_time = time.time() - start_time
    
    print(f"   首次签名时间: {first_sign_time:.6f}秒")
    print(f"   缓存签名时间: {second_sign_time:.6f}秒")
    print(f"   缓存加速比: {first_sign_time/second_sign_time:.2f}x")

def demo_signature_misuse():
    """演示签名算法误用"""
    print("\n" + "=" * 60)
    print("SM2签名算法误用演示")
    print("=" * 60)
    
    poc = SignatureMisusePOC()
    
    print("开始运行签名算法误用攻击演示...")
    print("注意: 这些演示仅用于安全研究和教育目的")
    
    # 运行所有攻击
    results = poc.run_all_attacks()
    
    print(f"\n攻击演示完成，共{len(results)}种攻击类型")

def demo_satoshi_forgery():
    """演示中本聪签名伪造"""
    print("\n" + "=" * 60)
    print("中本聪签名伪造演示")
    print("=" * 60)
    
    poc = SatoshiForgeryPOC()
    
    print("开始运行中本聪签名伪造攻击演示...")
    print("注意: 这些演示仅用于安全研究和教育目的")
    print("目标: 比特币创世区块消息")
    
    # 运行所有伪造攻击
    results = poc.run_all_forgery_attacks()
    
    print(f"\n伪造攻击演示完成，共{len(results)}种攻击类型")

def performance_comparison():
    """性能对比分析"""
    print("\n" + "=" * 60)
    print("SM2算法性能对比分析")
    print("=" * 60)
    
    # 测试消息
    test_messages = [
        b"Short message",
        b"Medium length message for testing",
        b"This is a longer message to test the performance of different SM2 implementations with various message lengths"
    ]
    
    # 不同实现
    implementations = {
        "基础版本": SM2(),
        "平衡优化": SM2Optimized("balanced"),
        "快速优化": SM2Optimized("fast"),
        "内存优化": SM2MemoryOptimized()
    }
    
    results = {}
    
    for name, impl in implementations.items():
        print(f"\n{name}:")
        results[name] = {}
        
        # 生成密钥对
        start_time = time.time()
        public_key, private_key = impl.generate_keypair()
        keygen_time = time.time() - start_time
        results[name]["密钥生成"] = keygen_time
        
        print(f"   密钥生成: {keygen_time:.6f}秒")
        
        # 测试不同长度消息
        for i, message in enumerate(test_messages):
            # 签名
            start_time = time.time()
            if hasattr(impl, 'sign_optimized'):
                signature = impl.sign_optimized(message, private_key)
            elif hasattr(impl, 'sign_memory_optimized'):
                signature = impl.sign_memory_optimized(message, private_key)
            else:
                signature = impl.sign(message, private_key)
            sign_time = time.time() - start_time
            
            # 验证
            start_time = time.time()
            if hasattr(impl, 'verify_optimized'):
                is_valid = impl.verify_optimized(message, signature, public_key)
            elif hasattr(impl, 'verify_memory_optimized'):
                is_valid = impl.verify_memory_optimized(message, signature, public_key)
            else:
                is_valid = impl.verify(message, signature, public_key)
            verify_time = time.time() - start_time
            
            print(f"   消息{i+1} ({len(message)}字节):")
            print(f"     签名: {sign_time:.6f}秒")
            print(f"     验证: {verify_time:.6f}秒")
            print(f"     验证结果: {'成功' if is_valid else '失败'}")
            
            if f"消息{i+1}" not in results[name]:
                results[name][f"消息{i+1}"] = {}
            results[name][f"消息{i+1}"]["签名"] = sign_time
            results[name][f"消息{i+1}"]["验证"] = verify_time
    
    # 生成性能报告
    print("\n" + "=" * 60)
    print("性能对比总结")
    print("=" * 60)
    
    for name, data in results.items():
        print(f"\n{name}:")
        avg_sign = sum(data[f"消息{i+1}"]["签名"] for i in range(len(test_messages))) / len(test_messages)
        avg_verify = sum(data[f"消息{i+1}"]["验证"] for i in range(len(test_messages))) / len(test_messages)
        print(f"   平均签名时间: {avg_sign:.6f}秒")
        print(f"   平均验证时间: {avg_verify:.6f}秒")
        print(f"   密钥生成时间: {data['密钥生成']:.6f}秒")

def main():
    """主函数"""
    print("SM2软件实现优化项目")
    print("作者: P5 Team")
    print("版本: 1.0.0")
    print("日期: 2024")
    
    try:
        # 基础功能演示
        demo_basic_sm2()
        
        # 优化版本演示
        demo_optimized_sm2()
        
        # 性能对比
        performance_comparison()
        
        # 询问是否运行安全演示
        print("\n" + "=" * 60)
        print("安全演示选项")
        print("=" * 60)
        print("以下演示包含安全漏洞和攻击方法，仅用于教育目的")
        print("1. 签名算法误用演示")
        print("2. 中本聪签名伪造演示")
        print("3. 跳过安全演示")
        
        while True:
            choice = input("\n请选择 (1-3): ").strip()
            if choice == "1":
                demo_signature_misuse()
                break
            elif choice == "2":
                demo_satoshi_forgery()
                break
            elif choice == "3":
                print("跳过安全演示")
                break
            else:
                print("无效选择，请输入1-3")
        
        print("\n" + "=" * 60)
        print("演示完成！")
        print("=" * 60)
        print("感谢使用SM2软件实现优化项目")
        print("如需运行测试，请执行: python -m pytest tests/")
        
    except KeyboardInterrupt:
        print("\n\n程序被用户中断")
    except Exception as e:
        print(f"\n程序执行出错: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
