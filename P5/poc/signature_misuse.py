"""
SM2签名算法误用POC验证
基于20250713-wen-sm2-public.pdf中提到的漏洞
"""

import hashlib
import random
from typing import Tuple, List, Dict
from ..sm2.core import SM2
from ..sm2.utils import Point, mod_inverse

class SignatureMisusePOC:
    """SM2签名算法误用概念验证"""
    
    def __init__(self):
        self.sm2 = SM2()
        self.vulnerabilities = {
            "weak_random": "弱随机数生成",
            "reused_nonce": "随机数重用",
            "parameter_manipulation": "参数操纵",
            "curve_manipulation": "曲线参数操纵",
            "hash_collision": "哈希碰撞攻击"
        }
    
    def demonstrate_weak_random_attack(self) -> Dict:
        """演示弱随机数攻击"""
        print("=== 弱随机数攻击演示 ===")
        
        # 生成密钥对
        public_key, private_key = self.sm2.generate_keypair()
        message = b"Hello, SM2!"
        
        # 使用弱随机数生成器
        def weak_random():
            return 12345  # 固定值
        
        # 模拟弱随机数签名
        k = weak_random()
        Z = self.sm2._compute_z("1234567890123456", public_key)
        e = self.sm2._hash_message(message, Z)
        
        point1 = self.sm2._fast_point_multiply(self.sm2.G, k)
        x1 = point1.x
        r = (e + x1) % self.sm2.n
        
        if r == 0 or r + k == self.sm2.n:
            print("弱随机数导致无效签名")
            return {"status": "failed", "reason": "invalid_signature"}
        
        s = ((1 + private_key) * mod_inverse(k - r * private_key, self.sm2.n)) % self.sm2.n
        signature = (r, s)
        
        # 验证签名
        is_valid = self.sm2.verify(message, signature, public_key)
        
        print(f"使用弱随机数k={k}")
        print(f"生成的签名: r={r}, s={s}")
        print(f"签名验证结果: {is_valid}")
        
        # 分析攻击
        if is_valid:
            print("⚠️  攻击成功: 弱随机数生成的签名仍然有效")
            print("攻击者可以通过枚举k值来恢复私钥")
        
        return {
            "status": "success" if is_valid else "failed",
            "k": k,
            "signature": signature,
            "is_valid": is_valid,
            "vulnerability": "weak_random"
        }
    
    def demonstrate_nonce_reuse_attack(self) -> Dict:
        """演示随机数重用攻击"""
        print("\n=== 随机数重用攻击演示 ===")
        
        # 生成密钥对
        public_key, private_key = self.sm2.generate_keypair()
        
        # 使用相同的随机数k签名两个不同消息
        k = random.randint(1, self.sm2.n - 1)
        message1 = b"Message 1"
        message2 = b"Message 2"
        
        # 签名第一个消息
        Z1 = self.sm2._compute_z("1234567890123456", public_key)
        e1 = self.sm2._hash_message(message1, Z1)
        
        point1_1 = self.sm2._fast_point_multiply(self.sm2.G, k)
        x1_1 = point1_1.x
        r1 = (e1 + x1_1) % self.sm2.n
        
        s1 = ((1 + private_key) * mod_inverse(k - r1 * private_key, self.sm2.n)) % self.sm2.n
        signature1 = (r1, s1)
        
        # 签名第二个消息
        Z2 = self.sm2._compute_z("1234567890123456", public_key)
        e2 = self.sm2._hash_message(message2, Z2)
        
        point1_2 = self.sm2._fast_point_multiply(self.sm2.G, k)
        x1_2 = point1_2.x
        r2 = (e2 + x1_2) % self.sm2.n
        
        s2 = ((1 + private_key) * mod_inverse(k - r2 * private_key, self.sm2.n)) % self.sm2.n
        signature2 = (r2, s2)
        
        print(f"使用相同随机数k={k}")
        print(f"消息1签名: r1={r1}, s1={s1}")
        print(f"消息2签名: r2={r2}, s2={s2}")
        
        # 验证签名
        valid1 = self.sm2.verify(message1, signature1, public_key)
        valid2 = self.sm2.verify(message2, signature2, public_key)
        
        print(f"签名1验证: {valid1}")
        print(f"签名2验证: {valid2}")
        
        # 演示私钥恢复
        if valid1 and valid2 and r1 != r2:
            print("⚠️  攻击成功: 可以通过随机数重用恢复私钥")
            
            # 计算私钥: d = (s1 - s2) / (r1 - r2) mod n
            try:
                recovered_private_key = ((s1 - s2) * mod_inverse(r1 - r2, self.sm2.n)) % self.sm2.n
                print(f"恢复的私钥: {recovered_private_key}")
                print(f"原始私钥: {private_key}")
                print(f"私钥恢复成功: {recovered_private_key == private_key}")
            except:
                print("私钥恢复失败")
        
        return {
            "status": "success",
            "k": k,
            "signature1": signature1,
            "signature2": signature2,
            "valid1": valid1,
            "valid2": valid2,
            "vulnerability": "reused_nonce"
        }
    
    def demonstrate_parameter_manipulation(self) -> Dict:
        """演示参数操纵攻击"""
        print("\n=== 参数操纵攻击演示 ===")
        
        # 创建恶意参数
        malicious_p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
        malicious_a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
        malicious_b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
        malicious_n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
        
        # 尝试构造弱曲线
        weak_b = 0  # 设置b=0使曲线更容易攻击
        
        print(f"原始参数: a={hex(malicious_a)}, b={hex(malicious_b)}")
        print(f"恶意参数: a={hex(malicious_a)}, b={hex(weak_b)}")
        
        # 检查曲线安全性
        security_level = self._analyze_curve_security(malicious_p, malicious_a, weak_b)
        print(f"曲线安全等级: {security_level}")
        
        if security_level == "weak":
            print("⚠️  攻击成功: 构造了弱曲线")
            print("弱曲线可能容易受到离散对数攻击")
        
        return {
            "status": "success",
            "original_params": {"a": malicious_a, "b": malicious_b},
            "malicious_params": {"a": malicious_a, "b": weak_b},
            "security_level": security_level,
            "vulnerability": "parameter_manipulation"
        }
    
    def demonstrate_hash_collision_attack(self) -> Dict:
        """演示哈希碰撞攻击"""
        print("\n=== 哈希碰撞攻击演示 ===")
        
        # 生成密钥对
        public_key, private_key = self.sm2.generate_keypair()
        
        # 寻找哈希碰撞
        target_hash = hashlib.sha256(b"target_message").digest()
        print(f"目标哈希: {target_hash.hex()}")
        
        # 尝试找到碰撞
        collision_found = False
        collision_message = None
        
        for i in range(10000):  # 限制搜索范围
            test_message = f"test_message_{i}".encode()
            test_hash = hashlib.sha256(test_message).digest()
            
            if test_hash == target_hash and test_message != b"target_message":
                collision_found = True
                collision_message = test_message
                break
        
        if collision_found:
            print("⚠️  攻击成功: 找到哈希碰撞")
            print(f"原始消息: target_message")
            print(f"碰撞消息: {collision_message.decode()}")
            print(f"碰撞哈希: {target_hash.hex()}")
            
            # 演示碰撞攻击
            original_signature = self.sm2.sign(b"target_message", private_key)
            collision_signature = self.sm2.sign(collision_message, private_key)
            
            print(f"原始消息签名: {original_signature}")
            print(f"碰撞消息签名: {collision_signature}")
            
            # 验证碰撞攻击
            valid_original = self.sm2.verify(b"target_message", original_signature, public_key)
            valid_collision = self.sm2.verify(collision_message, collision_signature, public_key)
            
            print(f"原始签名验证: {valid_original}")
            print(f"碰撞签名验证: {valid_collision}")
        
        else:
            print("未找到哈希碰撞")
        
        return {
            "status": "success" if collision_found else "failed",
            "collision_found": collision_found,
            "collision_message": collision_message,
            "target_hash": target_hash.hex(),
            "vulnerability": "hash_collision"
        }
    
    def demonstrate_curve_manipulation(self) -> Dict:
        """演示曲线操纵攻击"""
        print("\n=== 曲线操纵攻击演示 ===")
        
        # 尝试构造超奇异曲线
        p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
        
        # 检查p-1的因子
        factors = self._factorize(p - 1)
        print(f"p-1的因子: {factors}")
        
        # 检查是否存在小因子
        small_factors = [f for f in factors if f < 1000]
        if small_factors:
            print("⚠️  发现小因子: {small_factors}")
            print("这可能使曲线容易受到Pohlig-Hellman攻击")
        
        # 检查曲线阶数
        curve_order = self.sm2.n
        order_factors = self._factorize(curve_order)
        print(f"曲线阶数因子: {order_factors}")
        
        # 检查是否存在小因子
        small_order_factors = [f for f in order_factors if f < 1000]
        if small_order_factors:
            print("⚠️  曲线阶数存在小因子: {small_order_factors}")
            print("这可能使曲线容易受到离散对数攻击")
        
        return {
            "status": "success",
            "p_factors": factors,
            "order_factors": order_factors,
            "small_p_factors": small_factors,
            "small_order_factors": small_order_factors,
            "vulnerability": "curve_manipulation"
        }
    
    def run_all_attacks(self) -> Dict:
        """运行所有攻击演示"""
        print("开始运行SM2签名算法误用攻击演示...")
        
        results = {}
        
        # 弱随机数攻击
        results["weak_random"] = self.demonstrate_weak_random_attack()
        
        # 随机数重用攻击
        results["reused_nonce"] = self.demonstrate_nonce_reuse_attack()
        
        # 参数操纵攻击
        results["parameter_manipulation"] = self.demonstrate_parameter_manipulation()
        
        # 哈希碰撞攻击
        results["hash_collision"] = self.demonstrate_hash_collision_attack()
        
        # 曲线操纵攻击
        results["curve_manipulation"] = self.demonstrate_curve_manipulation()
        
        # 生成攻击报告
        self._generate_attack_report(results)
        
        return results
    
    def _analyze_curve_security(self, p: int, a: int, b: int) -> str:
        """分析曲线安全性"""
        # 简单的安全性检查
        if b == 0:
            return "weak"  # b=0的曲线较弱
        
        # 检查曲线阶数
        # 这里简化处理，实际应该计算完整的曲线阶数
        return "secure"
    
    def _factorize(self, n: int) -> List[int]:
        """简单的因子分解"""
        factors = []
        d = 2
        while d * d <= n:
            while n % d == 0:
                factors.append(d)
                n //= d
            d += 1
        if n > 1:
            factors.append(n)
        return factors
    
    def _generate_attack_report(self, results: Dict):
        """生成攻击报告"""
        print("\n" + "="*50)
        print("SM2签名算法误用攻击报告")
        print("="*50)
        
        total_attacks = len(results)
        successful_attacks = sum(1 for r in results.values() if r.get("status") == "success")
        
        print(f"总攻击数: {total_attacks}")
        print(f"成功攻击数: {successful_attacks}")
        print(f"成功率: {successful_attacks/total_attacks*100:.1f}%")
        
        print("\n详细结果:")
        for attack_name, result in results.items():
            status = result.get("status", "unknown")
            vulnerability = result.get("vulnerability", "unknown")
            print(f"- {attack_name}: {status} ({vulnerability})")
        
        print("\n安全建议:")
        print("1. 使用密码学安全的随机数生成器")
        print("2. 确保每次签名使用不同的随机数")
        print("3. 验证所有曲线参数")
        print("4. 使用标准的哈希函数")
        print("5. 定期更新密钥")
        
        print("="*50)
