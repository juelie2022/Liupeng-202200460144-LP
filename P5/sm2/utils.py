"""
SM2算法工具函数
包含椭圆曲线点运算和数学函数
"""

import hashlib
import random
from typing import Tuple, Optional
from dataclasses import dataclass

@dataclass
class Point:
    """椭圆曲线上的点"""
    x: int
    y: int
    
    def __eq__(self, other):
        if not isinstance(other, Point):
            return False
        return self.x == other.x and self.y == other.y
    
    def __repr__(self):
        return f"Point({hex(self.x)}, {hex(self.y)})"

# 无穷远点
INFINITY = Point(0, 0)

def mod_inverse(a: int, m: int) -> int:
    """计算模逆: a^(-1) mod m"""
    def extended_gcd(a: int, b: int) -> Tuple[int, int, int]:
        if a == 0:
            return b, 0, 1
        gcd, x1, y1 = extended_gcd(b % a, a)
        x = y1 - (b // a) * x1
        y = x1
        return gcd, x, y
    
    gcd, x, _ = extended_gcd(a, m)
    if gcd != 1:
        raise ValueError("模逆不存在")
    
    return (x % m + m) % m

def mod_sqrt(a: int, p: int) -> Optional[int]:
    """计算模平方根: sqrt(a) mod p (p是素数)"""
    if a == 0:
        return 0
    
    # 使用Tonelli-Shanks算法
    if p % 4 == 3:
        return pow(a, (p + 1) // 4, p)
    
    # 对于其他素数，使用更复杂的算法
    def legendre_symbol(a: int, p: int) -> int:
        ls = pow(a, (p - 1) // 2, p)
        return -1 if ls == p - 1 else ls
    
    if legendre_symbol(a, p) != 1:
        return None
    
    # 找到二次非剩余
    q = p - 1
    s = 0
    while q % 2 == 0:
        q //= 2
        s += 1
    
    z = 2
    while legendre_symbol(z, p) != -1:
        z += 1
    
    m = s
    c = pow(z, q, p)
    t = pow(a, q, p)
    r = pow(a, (q + 1) // 2, p)
    
    while t != 1:
        i = 0
        temp = t
        while temp != 1 and i < m:
            temp = (temp * temp) % p
            i += 1
        
        if i == 0:
            return r
        
        b = pow(c, 2 ** (m - i - 1), p)
        m = i
        c = (b * b) % p
        t = (t * c) % p
        r = (r * b) % p
    
    return r

def point_add(P: Point, Q: Point, p: int, a: int) -> Point:
    """椭圆曲线点加法: P + Q"""
    if P == INFINITY:
        return Q
    if Q == INFINITY:
        return P
    
    if P.x == Q.x and P.y != Q.y:
        return INFINITY
    
    if P.x == Q.x:
        # P = Q，计算切线斜率
        if P.y == 0:
            return INFINITY
        lam = ((3 * P.x * P.x + a) * mod_inverse(2 * P.y, p)) % p
    else:
        # P ≠ Q，计算连线斜率
        lam = ((Q.y - P.y) * mod_inverse(Q.x - P.x, p)) % p
    
    x3 = (lam * lam - P.x - Q.x) % p
    y3 = (lam * (P.x - x3) - P.y) % p
    
    return Point(x3, y3)

def point_multiply(P: Point, k: int, p: int, a: int) -> Point:
    """椭圆曲线标量乘法: k * P (使用double-and-add算法)"""
    if k == 0:
        return INFINITY
    
    result = INFINITY
    addend = P
    
    while k > 0:
        if k & 1:
            result = point_add(result, addend, p, a)
        addend = point_add(addend, addend, p, a)
        k >>= 1
    
    return result

def generate_random_point(p: int, a: int, b: int) -> Point:
    """在椭圆曲线上生成随机点"""
    while True:
        x = random.randint(0, p - 1)
        # 计算 y^2 = x^3 + ax + b
        y_squared = (x * x * x + a * x + b) % p
        
        # 尝试计算平方根
        y = mod_sqrt(y_squared, p)
        if y is not None:
            return Point(x, y)

def hash_to_point(message: bytes, p: int, a: int, b: int) -> Point:
    """将消息哈希到椭圆曲线上的点"""
    hash_value = hashlib.sha256(message).digest()
    x = int.from_bytes(hash_value, 'big') % p
    
    # 尝试找到对应的y值
    for i in range(100):  # 最多尝试100次
        x_candidate = (x + i) % p
        y_squared = (x_candidate * x_candidate * x_candidate + 
                    a * x_candidate + b) % p
        
        y = mod_sqrt(y_squared, p)
        if y is not None:
            return Point(x_candidate, y)
    
    # 如果找不到，返回生成元点
    return Point(0, 0)

def is_point_on_curve(point: Point, p: int, a: int, b: int) -> bool:
    """检查点是否在椭圆曲线上"""
    if point == INFINITY:
        return True
    
    left = (point.y * point.y) % p
    right = (point.x * point.x * point.x + a * point.x + b) % p
    return left == right

def generate_random_scalar(n: int) -> int:
    """生成随机标量 (1 <= k < n)"""
    return random.randint(1, n - 1)

def bytes_to_point(data: bytes) -> Point:
    """将字节数据转换为点"""
    if len(data) < 64:
        raise ValueError("数据长度不足")
    
    x = int.from_bytes(data[:32], 'big')
    y = int.from_bytes(data[32:64], 'big')
    return Point(x, y)

def point_to_bytes(point: Point) -> bytes:
    """将点转换为字节数据"""
    return point.x.to_bytes(32, 'big') + point.y.to_bytes(32, 'big')

def int_to_bytes(value: int, length: int = None) -> bytes:
    """将整数转换为字节"""
    if length is None:
        # 计算最小长度
        length = (value.bit_length() + 7) // 8
        if length == 0:
            length = 1
    
    return value.to_bytes(length, 'big')

def bytes_to_int(data: bytes) -> int:
    """将字节转换为整数"""
    return int.from_bytes(data, 'big')

def pad_message(message: bytes, block_size: int = 32) -> bytes:
    """填充消息到指定块大小"""
    padding_length = block_size - (len(message) % block_size)
    if padding_length == 0:
        padding_length = block_size
    
    padding = bytes([padding_length] * padding_length)
    return message + padding

def unpad_message(padded_message: bytes, block_size: int = 32) -> bytes:
    """移除消息填充"""
    if len(padded_message) == 0:
        return b''
    
    padding_length = padded_message[-1]
    if padding_length > block_size or padding_length == 0:
        raise ValueError("无效的填充")
    
    return padded_message[:-padding_length]
