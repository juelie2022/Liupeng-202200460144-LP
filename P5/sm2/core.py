"""
SM2椭圆曲线密码算法基础实现
基于国密标准GM/T 0003-2012
"""

import hashlib
import random
from typing import Tuple, Optional
from .utils import (
    Point, INFINITY, 
    point_add, point_multiply, 
    generate_random_point, hash_to_point,
    mod_inverse, mod_sqrt
)

class SM2:
    """SM2椭圆曲线密码算法实现"""
    
    def __init__(self):
        # SM2推荐参数 (来自国密标准)
        self.p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
        self.a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
        self.b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
        self.n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
        self.G = Point(
            0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7,
            0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
        )
        
        # 预计算值
        self.h = 1  # 余因子
        
    def generate_keypair(self) -> Tuple[Point, int]:
        """生成SM2密钥对"""
        while True:
            # 生成随机私钥
            private_key = random.randint(1, self.n - 1)
            
            # 计算公钥
            public_key = point_multiply(self.G, private_key, self.p, self.a)
            
            if public_key != INFINITY:
                return public_key, private_key
    
    def sign(self, message: bytes, private_key: int, 
             user_id: str = "1234567890123456") -> Tuple[int, int]:
        """
        SM2数字签名
        
        Args:
            message: 待签名消息
            private_key: 私钥
            user_id: 用户ID (默认16字节)
            
        Returns:
            (r, s) 签名值
        """
        # 计算Z值 (用户公钥的哈希)
        public_key = point_multiply(self.G, private_key, self.p, self.a)
        Z = self._compute_z(user_id, public_key)
        
        # 计算e值 (消息哈希)
        e = self._hash_message(message, Z)
        
        while True:
            # 生成随机数k
            k = random.randint(1, self.n - 1)
            
            # 计算点(x1, y1) = k * G
            point1 = point_multiply(self.G, k, self.p, self.a)
            x1 = point1.x
            
            # 计算r = (e + x1) mod n
            r = (e + x1) % self.n
            if r == 0 or r + k == self.n:
                continue
                
            # 计算s = ((1 + d)^-1 * (k - r * d)) mod n
            s = ((1 + private_key) * mod_inverse(k - r * private_key, self.n)) % self.n
            if s == 0:
                continue
                
            return r, s
    
    def verify(self, message: bytes, signature: Tuple[int, int], 
               public_key: Point, user_id: str = "1234567890123456") -> bool:
        """
        验证SM2数字签名
        
        Args:
            message: 原始消息
            signature: 签名值 (r, s)
            public_key: 公钥
            user_id: 用户ID
            
        Returns:
            签名是否有效
        """
        r, s = signature
        
        # 验证签名值范围
        if not (1 <= r <= self.n - 1 and 1 <= s <= self.n - 1):
            return False
            
        # 计算Z值
        Z = self._compute_z(user_id, public_key)
        
        # 计算e值
        e = self._hash_message(message, Z)
        
        # 计算t = (r + s) mod n
        t = (r + s) % self.n
        if t == 0:
            return False
            
        # 计算点(x1, y1) = s * G + t * P
        point1 = point_multiply(self.G, s, self.p, self.a)
        point2 = point_multiply(public_key, t, self.p, self.a)
        point3 = point_add(point1, point2, self.p, self.a)
        
        if point3 == INFINITY:
            return False
            
        # 计算R = (e + x1) mod n
        R = (e + point3.x) % self.n
        
        return R == r
    
    def encrypt(self, message: bytes, public_key: Point) -> bytes:
        """
        SM2加密
        
        Args:
            message: 待加密消息
            public_key: 公钥
            
        Returns:
            密文
        """
        # 生成随机数k
        k = random.randint(1, self.n - 1)
        
        # 计算C1 = k * G
        C1 = point_multiply(self.G, k, self.p, self.a)
        
        # 计算k * P
        kP = point_multiply(public_key, k, self.p, self.a)
        
        # 计算t = KDF(kP, len(message))
        t = self._kdf(kP, len(message))
        
        # 计算C2 = message XOR t
        C2 = bytes(a ^ b for a, b in zip(message, t))
        
        # 计算C3 = Hash(kP || message)
        C3 = hashlib.sha256(kP.x.to_bytes(32, 'big') + 
                           kP.y.to_bytes(32, 'big') + message).digest()
        
        # 返回密文 C1 || C2 || C3
        return (C1.x.to_bytes(32, 'big') + 
                C1.y.to_bytes(32, 'big') + 
                C2 + C3)
    
    def decrypt(self, ciphertext: bytes, private_key: int) -> Optional[bytes]:
        """
        SM2解密
        
        Args:
            ciphertext: 密文
            private_key: 私钥
            
        Returns:
            明文，如果解密失败返回None
        """
        if len(ciphertext) < 96:  # C1(64) + C3(32) + 至少1字节C2
            return None
            
        # 解析密文
        C1_x = int.from_bytes(ciphertext[:32], 'big')
        C1_y = int.from_bytes(ciphertext[32:64], 'big')
        C1 = Point(C1_x, C1_y)
        
        # 验证C1是否在曲线上
        if not self._is_point_on_curve(C1):
            return None
            
        # 计算d * C1
        dC1 = point_multiply(C1, private_key, self.p, self.a)
        
        # 计算t = KDF(dC1, len(C2))
        C2_len = len(ciphertext) - 96
        t = self._kdf(dC1, C2_len)
        
        # 计算C2
        C2 = ciphertext[64:64+C2_len]
        
        # 计算message = C2 XOR t
        message = bytes(a ^ b for a, b in zip(C2, t))
        
        # 验证C3
        C3 = ciphertext[64+C2_len:]
        expected_C3 = hashlib.sha256(dC1.x.to_bytes(32, 'big') + 
                                   dC1.y.to_bytes(32, 'big') + message).digest()
        
        if C3 != expected_C3:
            return None
            
        return message
    
    def _compute_z(self, user_id: str, public_key: Point) -> bytes:
        """计算Z值"""
        # Z = Hash(ENTL || ID || a || b || xG || yG || xA || yA)
        entl = len(user_id.encode()) * 8
        data = (entl.to_bytes(2, 'big') + 
                user_id.encode() + 
                self.a.to_bytes(32, 'big') + 
                self.b.to_bytes(32, 'big') + 
                self.G.x.to_bytes(32, 'big') + 
                self.G.y.to_bytes(32, 'big') + 
                public_key.x.to_bytes(32, 'big') + 
                public_key.y.to_bytes(32, 'big'))
        return hashlib.sha256(data).digest()
    
    def _hash_message(self, message: bytes, Z: bytes) -> int:
        """计算消息哈希值"""
        data = Z + message
        hash_value = hashlib.sha256(data).digest()
        return int.from_bytes(hash_value, 'big')
    
    def _kdf(self, point: Point, length: int) -> bytes:
        """密钥派生函数"""
        result = b''
        counter = 1
        
        while len(result) < length:
            data = (point.x.to_bytes(32, 'big') + 
                   point.y.to_bytes(32, 'big') + 
                   counter.to_bytes(4, 'big'))
            hash_value = hashlib.sha256(data).digest()
            result += hash_value
            counter += 1
            
        return result[:length]
    
    def _is_point_on_curve(self, point: Point) -> bool:
        """检查点是否在曲线上"""
        if point == INFINITY:
            return True
        left = (point.y * point.y) % self.p
        right = (point.x * point.x * point.x + 
                self.a * point.x + self.b) % self.p
        return left == right
