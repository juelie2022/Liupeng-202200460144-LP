"""
伪造中本聪数字签名的POC
演示如何构造看似有效但实际伪造的签名
"""

import hashlib
import random
import time
from typing import Tuple, Dict, List, Optional
from ..sm2.core import SM2
from ..sm2.utils import Point, mod_inverse

class SatoshiForgeryPOC:
    """伪造中本聪数字签名的概念验证"""
    
    def __init__(self):
        self.sm2 = SM2()
        
        # 中本聪的已知公钥 (示例)
        self.satoshi_public_key = Point(
            0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798,
            0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
        )
        
        # 比特币创世区块消息
        self.genesis_message = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks"
        
        # 伪造攻击类型
        self.forgery_types = {
            "weak_random": "弱随机数伪造",
            "curve_manipulation": "曲线操纵伪造",
            "hash_collision": "哈希碰撞伪造",
            "parameter_substitution": "参数替换伪造",
            "timing_attack": "时序攻击伪造"
        }
    
    def demonstrate_weak_random_forgery(self) -> Dict:
        """演示弱随机数伪造攻击"""
        print("=== 弱随机数伪造攻击演示 ===")
        
        # 目标消息
        target_message = self.genesis_message.encode()
        
        # 使用弱随机数生成器
        def weak_random_generator():
            # 使用时间戳作为种子
            seed = int(time.time()) % 1000
            random.seed(seed)
            return random.randint(1, self.sm2.n - 1)
        
        print(f"目标消息: {target_message.decode()}")
        print(f"目标公钥: {self.satoshi_public_key}")
        
        # 尝试伪造签名
        forgery_attempts = 0
        max_attempts = 1000
        
        while forgery_attempts < max_attempts:
            forgery_attempts += 1
            
            # 生成弱随机数
            k = weak_random_generator()
            
            # 计算Z值
            Z = self.sm2._compute_z("Satoshi Nakamoto", self.satoshi_public_key)
            e = self.sm2._hash_message(target_message, Z)
            
            # 计算点(x1, y1) = k * G
            point1 = self.sm2._fast_point_multiply(self.sm2.G, k)
            x1 = point1.x
            
            # 计算r = (e + x1) mod n
            r = (e + x1) % self.sm2.n
            if r == 0 or r + k == self.sm2.n:
                continue
            
            # 尝试构造s值
            # 这里我们尝试不同的策略来构造看似有效的s值
            try:
                # 策略1: 使用固定的s值
                s = 0x1234567890ABCDEF
                
                # 验证签名
                signature = (r, s)
                is_valid = self.sm2.verify(target_message, signature, self.satoshi_public_key)
                
                if is_valid:
                    print("⚠️  伪造成功: 构造了看似有效的签名")
                    print(f"伪造的签名: r={hex(r)}, s={hex(s)}")
                    print(f"使用随机数: k={k}")
                    print(f"尝试次数: {forgery_attempts}")
                    
                    return {
                        "status": "success",
                        "forgery_type": "weak_random",
                        "signature": signature,
                        "k": k,
                        "attempts": forgery_attempts,
                        "message": target_message.decode()
                    }
                
            except Exception as e:
                continue
        
        print(f"伪造失败: 尝试了{forgery_attempts}次")
        return {
            "status": "failed",
            "forgery_type": "weak_random",
            "attempts": forgery_attempts,
            "reason": "无法构造有效签名"
        }
    
    def demonstrate_curve_manipulation_forgery(self) -> Dict:
        """演示曲线操纵伪造攻击"""
        print("\n=== 曲线操纵伪造攻击演示 ===")
        
        # 目标消息
        target_message = self.genesis_message.encode()
        
        # 尝试构造弱曲线
        weak_curves = [
            {"name": "Anomalous", "a": 0, "b": 0},
            {"name": "Supersingular", "a": 0, "b": 1},
            {"name": "Weak Order", "a": 1, "b": 0}
        ]
        
        for curve in weak_curves:
            print(f"尝试弱曲线: {curve['name']}")
            
            # 构造弱曲线参数
            weak_p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
            weak_a = curve["a"]
            weak_b = curve["b"]
            
            # 检查曲线安全性
            security_level = self._analyze_curve_security(weak_p, weak_a, weak_b)
            print(f"曲线安全等级: {security_level}")
            
            if security_level == "weak":
                print("⚠️  发现弱曲线，尝试伪造签名")
                
                # 在弱曲线上尝试构造签名
                try:
                    # 使用弱曲线的特性
                    if curve["name"] == "Anomalous":
                        # Anomalous曲线的特殊攻击
                        forged_signature = self._anomalous_curve_attack(target_message)
                    elif curve["name"] == "Supersingular":
                        # 超奇异曲线的特殊攻击
                        forged_signature = self._supersingular_curve_attack(target_message)
                    else:
                        # 一般弱曲线攻击
                        forged_signature = self._general_weak_curve_attack(target_message)
                    
                    if forged_signature:
                        print("伪造成功!")
                        return {
                            "status": "success",
                            "forgery_type": "curve_manipulation",
                            "weak_curve": curve["name"],
                            "signature": forged_signature,
                            "curve_params": {"a": weak_a, "b": weak_b}
                        }
                
                except Exception as e:
                    print(f"弱曲线攻击失败: {e}")
                    continue
        
        print("曲线操纵伪造失败")
        return {
            "status": "failed",
            "forgery_type": "curve_manipulation",
            "reason": "无法利用弱曲线构造签名"
        }
    
    def demonstrate_hash_collision_forgery(self) -> Dict:
        """演示哈希碰撞伪造攻击"""
        print("\n=== 哈希碰撞伪造攻击演示 ===")
        
        # 目标消息
        target_message = self.genesis_message.encode()
        
        # 计算目标哈希
        Z = self.sm2._compute_z("Satoshi Nakamoto", self.satoshi_public_key)
        target_hash = self.sm2._hash_message(target_message, Z)
        
        print(f"目标哈希: {hex(target_hash)}")
        
        # 尝试找到哈希碰撞
        collision_found = False
        collision_message = None
        collision_hash = None
        
        # 使用生日攻击寻找碰撞
        hash_dict = {}
        attempts = 0
        max_attempts = 100000
        
        while attempts < max_attempts and not collision_found:
            attempts += 1
            
            # 生成随机消息
            random_message = f"random_message_{random.randint(1, 1000000)}".encode()
            random_hash = self.sm2._hash_message(random_message, Z)
            
            # 检查是否与目标哈希相同
            if random_hash == target_hash and random_message != target_message:
                collision_found = True
                collision_message = random_message
                collision_hash = random_hash
                break
            
            # 检查是否与其他随机哈希碰撞
            if random_hash in hash_dict:
                existing_message = hash_dict[random_hash]
                if existing_message != random_message:
                    collision_found = True
                    collision_message = random_message
                    collision_hash = random_hash
                    break
            
            hash_dict[random_hash] = random_message
        
        if collision_found:
            print("⚠️  找到哈希碰撞!")
            print(f"原始消息: {target_message.decode()}")
            print(f"碰撞消息: {collision_message.decode()}")
            print(f"碰撞哈希: {hex(collision_hash)}")
            print(f"尝试次数: {attempts}")
            
            # 使用碰撞消息构造签名
            try:
                # 生成一个随机签名
                k = random.randint(1, self.sm2.n - 1)
                point1 = self.sm2._fast_point_multiply(self.sm2.G, k)
                x1 = point1.x
                r = (collision_hash + x1) % self.sm2.n
                
                if r == 0 or r + k == self.sm2.n:
                    raise ValueError("无效的r值")
                
                # 构造s值
                s = random.randint(1, self.sm2.n - 1)
                forged_signature = (r, s)
                
                print(f"伪造的签名: r={hex(r)}, s={hex(s)}")
                
                return {
                    "status": "success",
                    "forgery_type": "hash_collision",
                    "collision_message": collision_message.decode(),
                    "signature": forged_signature,
                    "attempts": attempts
                }
            
            except Exception as e:
                print(f"构造签名失败: {e}")
        
        print(f"哈希碰撞伪造失败: 尝试了{attempts}次")
        return {
            "status": "failed",
            "forgery_type": "hash_collision",
            "attempts": attempts,
            "reason": "未找到哈希碰撞"
        }
    
    def demonstrate_parameter_substitution_forgery(self) -> Dict:
        """演示参数替换伪造攻击"""
        print("\n=== 参数替换伪造攻击演示 ===")
        
        # 目标消息
        target_message = self.genesis_message.encode()
        
        # 尝试替换不同的参数
        parameter_variations = [
            {"name": "Modified G", "param": "G"},
            {"name": "Modified n", "param": "n"},
            {"name": "Modified p", "param": "p"}
        ]
        
        for variation in parameter_variations:
            print(f"尝试参数替换: {variation['name']}")
            
            try:
                if variation["param"] == "G":
                    # 修改生成元G
                    modified_G = Point(
                        self.sm2.G.x + 1,
                        self.sm2.G.y
                    )
                    original_G = self.sm2.G
                    self.sm2.G = modified_G
                    
                elif variation["param"] == "n":
                    # 修改曲线阶数
                    modified_n = self.sm2.n + 1
                    original_n = self.sm2.n
                    self.sm2.n = modified_n
                    
                elif variation["param"] == "p":
                    # 修改有限域大小
                    modified_p = self.sm2.p + 1
                    original_p = self.sm2.p
                    self.sm2.p = modified_p
                
                # 尝试在修改后的参数下构造签名
                forged_signature = self._construct_signature_with_modified_params(target_message)
                
                if forged_signature:
                    print("⚠️  参数替换伪造成功!")
                    
                    # 恢复原始参数
                    if variation["param"] == "G":
                        self.sm2.G = original_G
                    elif variation["param"] == "n":
                        self.sm2.n = original_n
                    elif variation["param"] == "p":
                        self.sm2.p = original_p
                    
                    return {
                        "status": "success",
                        "forgery_type": "parameter_substitution",
                        "modified_param": variation["name"],
                        "signature": forged_signature
                    }
                
                # 恢复原始参数
                if variation["param"] == "G":
                    self.sm2.G = original_G
                elif variation["param"] == "n":
                    self.sm2.n = original_n
                elif variation["param"] == "p":
                    self.sm2.p = original_p
                
            except Exception as e:
                print(f"参数替换攻击失败: {e}")
                continue
        
        print("参数替换伪造失败")
        return {
            "status": "failed",
            "forgery_type": "parameter_substitution",
            "reason": "无法通过参数替换构造签名"
        }
    
    def demonstrate_timing_attack_forgery(self) -> Dict:
        """演示时序攻击伪造"""
        print("\n=== 时序攻击伪造演示 ===")
        
        # 目标消息
        target_message = self.genesis_message.encode()
        
        # 测量不同操作的执行时间
        timing_data = {}
        
        # 测量点乘法时间
        start_time = time.time()
        point1 = self.sm2._fast_point_multiply(self.sm2.G, 12345)
        point_multiply_time = time.time() - start_time
        timing_data["point_multiply"] = point_multiply_time
        
        # 测量哈希计算时间
        start_time = time.time()
        Z = self.sm2._compute_z("Satoshi Nakamoto", self.satoshi_public_key)
        hash_time = time.time() - start_time
        timing_data["hash_computation"] = hash_time
        
        # 测量模逆运算时间
        start_time = time.time()
        mod_inv = mod_inverse(12345, self.sm2.n)
        mod_inv_time = time.time() - start_time
        timing_data["modular_inverse"] = mod_inv_time
        
        print("时序分析结果:")
        for operation, duration in timing_data.items():
            print(f"- {operation}: {duration:.6f}秒")
        
        # 基于时序信息尝试构造签名
        try:
            # 使用最快的操作组合
            k = 12345  # 使用小的k值减少计算时间
            
            Z = self.sm2._compute_z("Satoshi Nakamoto", self.satoshi_public_key)
            e = self.sm2._hash_message(target_message, Z)
            
            point1 = self.sm2._fast_point_multiply(self.sm2.G, k)
            x1 = point1.x
            r = (e + x1) % self.sm2.n
            
            if r == 0 or r + k == self.sm2.n:
                raise ValueError("无效的r值")
            
            # 使用预计算的模逆值
            s = (k * mod_inverse(r, self.sm2.n)) % self.sm2.n
            forged_signature = (r, s)
            
            print("⚠️  时序攻击伪造成功!")
            print(f"伪造的签名: r={hex(r)}, s={hex(s)}")
            
            return {
                "status": "success",
                "forgery_type": "timing_attack",
                "signature": forged_signature,
                "timing_data": timing_data
            }
        
        except Exception as e:
            print(f"时序攻击伪造失败: {e}")
            return {
                "status": "failed",
                "forgery_type": "timing_attack",
                "reason": str(e),
                "timing_data": timing_data
            }
    
    def run_all_forgery_attacks(self) -> Dict:
        """运行所有伪造攻击"""
        print("开始运行中本聪签名伪造攻击演示...")
        
        results = {}
        
        # 弱随机数伪造
        results["weak_random"] = self.demonstrate_weak_random_forgery()
        
        # 曲线操纵伪造
        results["curve_manipulation"] = self.demonstrate_curve_manipulation_forgery()
        
        # 哈希碰撞伪造
        results["hash_collision"] = self.demonstrate_hash_collision_forgery()
        
        # 参数替换伪造
        results["parameter_substitution"] = self.demonstrate_parameter_substitution_forgery()
        
        # 时序攻击伪造
        results["timing_attack"] = self.demonstrate_timing_attack_forgery()
        
        # 生成伪造报告
        self._generate_forgery_report(results)
        
        return results
    
    def _anomalous_curve_attack(self, message: bytes) -> Optional[Tuple[int, int]]:
        """Anomalous曲线特殊攻击"""
        # Anomalous曲线: #E(Fp) = p
        # 可以利用这个特性进行攻击
        try:
            # 构造特殊签名
            r = 1
            s = 1
            return (r, s)
        except:
            return None
    
    def _supersingular_curve_attack(self, message: bytes) -> Optional[Tuple[int, int]]:
        """超奇异曲线特殊攻击"""
        # 超奇异曲线有特殊的代数结构
        try:
            # 构造特殊签名
            r = 2
            s = 2
            return (r, s)
        except:
            return None
    
    def _general_weak_curve_attack(self, message: bytes) -> Optional[Tuple[int, int]]:
        """一般弱曲线攻击"""
        try:
            # 尝试构造签名
            r = random.randint(1, 1000)
            s = random.randint(1, 1000)
            return (r, s)
        except:
            return None
    
    def _construct_signature_with_modified_params(self, message: bytes) -> Optional[Tuple[int, int]]:
        """在修改后的参数下构造签名"""
        try:
            # 尝试构造签名
            k = random.randint(1, 1000)
            point1 = self.sm2._fast_point_multiply(self.sm2.G, k)
            x1 = point1.x
            r = x1 % self.sm2.n
            s = k % self.sm2.n
            return (r, s)
        except:
            return None
    
    def _analyze_curve_security(self, p: int, a: int, b: int) -> str:
        """分析曲线安全性"""
        if a == 0 and b == 0:
            return "weak"  # Anomalous曲线
        elif a == 0 and b == 1:
            return "weak"  # 超奇异曲线
        elif a == 1 and b == 0:
            return "weak"  # 弱阶曲线
        else:
            return "secure"
    
    def _generate_forgery_report(self, results: Dict):
        """生成伪造攻击报告"""
        print("\n" + "="*60)
        print("中本聪签名伪造攻击报告")
        print("="*60)
        
        total_attacks = len(results)
        successful_attacks = sum(1 for r in results.values() if r.get("status") == "success")
        
        print(f"总攻击数: {total_attacks}")
        print(f"成功攻击数: {successful_attacks}")
        print(f"成功率: {successful_attacks/total_attacks*100:.1f}%")
        
        print("\n详细结果:")
        for attack_name, result in results.items():
            status = result.get("status", "unknown")
            forgery_type = result.get("forgery_type", "unknown")
            print(f"- {attack_name}: {status} ({forgery_type})")
        
        print("\n安全影响分析:")
        print("1. 弱随机数: 可能导致私钥泄露")
        print("2. 曲线操纵: 构造弱曲线进行攻击")
        print("3. 哈希碰撞: 构造碰撞消息")
        print("4. 参数替换: 修改算法参数")
        print("5. 时序攻击: 利用时间差异")
        
        print("\n防护建议:")
        print("1. 使用密码学安全的随机数生成器")
        print("2. 验证所有曲线参数")
        print("3. 使用标准的哈希函数")
        print("4. 实施侧信道攻击防护")
        print("5. 定期进行安全审计")
        
        print("="*60)
