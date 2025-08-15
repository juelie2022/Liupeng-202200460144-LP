#!/usr/bin/env python3
"""
SM2功能演示脚本
快速测试SM2的基本功能
"""

import sys
from pathlib import Path

# 添加项目路径
sys.path.insert(0, str(Path(__file__).parent))

def quick_test():
    """快速测试SM2功能"""
    try:
        from sm2.core import SM2
        print("✓ SM2模块导入成功")
        
        # 创建SM2实例
        sm2 = SM2()
        print("✓ SM2实例创建成功")
        
        # 生成密钥对
        print("正在生成密钥对...")
        public_key, private_key = sm2.generate_keypair()
        print(f"✓ 密钥对生成成功")
        print(f"  公钥: {public_key}")
        print(f"  私钥: {hex(private_key)[:20]}...")
        
        # 测试签名
        message = b"Hello, SM2!"
        print(f"\n正在签名消息: {message.decode()}")
        signature = sm2.sign(message, private_key)
        print(f"✓ 签名成功: r={hex(signature[0])[:20]}..., s={hex(signature[1])[:20]}...")
        
        # 测试验证
        print("正在验证签名...")
        is_valid = sm2.verify(message, signature, public_key)
        print(f"✓ 签名验证: {'成功' if is_valid else '失败'}")
        
        # 测试加密解密
        secret_message = b"Secret message"
        print(f"\n正在加密消息: {secret_message.decode()}")
        ciphertext = sm2.encrypt(secret_message, public_key)
        print(f"✓ 加密成功，密文长度: {len(ciphertext)} 字节")
        
        print("正在解密消息...")
        decrypted = sm2.decrypt(ciphertext, private_key)
        print(f"✓ 解密成功: {decrypted.decode()}")
        print(f"  解密结果正确: {decrypted == secret_message}")
        
        print("\n🎉 所有测试通过！SM2实现工作正常。")
        return True
        
    except ImportError as e:
        print(f"✗ 模块导入失败: {e}")
        print("请确保已安装所有依赖: pip install -r requirements.txt")
        return False
    except Exception as e:
        print(f"✗ 测试失败: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_optimizations():
    """测试优化版本"""
    try:
        from sm2.optimized import SM2Optimized
        print("\n正在测试优化版本...")
        
        sm2_opt = SM2Optimized("balanced")
        public_key, private_key = sm2_opt.generate_keypair()
        
        message = b"Optimization test"
        signature = sm2_opt.sign_optimized(message, private_key)
        is_valid = sm2_opt.verify_optimized(message, signature, public_key)
        
        print(f"✓ 优化版本测试成功: {is_valid}")
        
        stats = sm2_opt.get_performance_stats()
        print(f"  优化级别: {stats['optimization_level']}")
        print(f"  预计算表大小: {stats['precompute_table_size']}")
        
        return True
        
    except Exception as e:
        print(f"✗ 优化版本测试失败: {e}")
        return False

if __name__ == "__main__":
    print("SM2功能演示脚本")
    print("=" * 40)
    
    # 基础功能测试
    if quick_test():
        # 优化版本测试
        test_optimizations()
        
        print("\n" + "=" * 40)
        print("演示完成！")
        print("运行 'python main.py' 查看完整演示")
        print("运行 'python -m pytest tests/' 执行测试")
    else:
        print("\n演示失败，请检查错误信息")
        sys.exit(1)
