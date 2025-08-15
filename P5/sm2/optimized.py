"""
SM2算法优化版本
包含多种优化策略的性能对比
"""

import time
import hashlib
import random
from typing import Tuple, Optional, List
from .core import SM2
from .utils import Point, INFINITY, point_add, point_multiply, mod_inverse

class SM2Optimized(SM2):
    """SM2算法优化版本"""
    
    def __init__(self, optimization_level: str = "balanced"):
        super().__init__()
        self.optimization_level = optimization_level
        
        # 预计算表
        self._precompute_table = {}
        self._build_precompute_table()
    
    def _build_precompute_table(self):
        """构建预计算表用于快速点乘法"""
        if self.optimization_level in ["fast", "balanced"]:
            # 预计算 2^i * G 和 3^i * G
            self._precompute_table = {}
            
            # 2^i * G
            current = self.G
            for i in range(256):
                self._precompute_table[f"2^{i}"] = current
                current = point_add(current, current, self.p, self.a)
            
            # 3^i * G (用于NAF优化)
            if self.optimization_level == "fast":
                current = self.G
                for i in range(128):
                    self._precompute_table[f"3^{i}"] = current
                    current = point_add(current, point_add(current, current, self.p, self.a), self.p, self.a)
    
    def _fast_point_multiply(self, P: Point, k: int) -> Point:
        """快速点乘法 (使用预计算表)"""
        if self.optimization_level == "fast":
            return self._naf_multiply(P, k)
        elif self.optimization_level == "balanced":
            return self._window_multiply(P, k)
        else:
            return point_multiply(P, k, self.p, self.a)
    
    def _naf_multiply(self, P: Point, k: int) -> Point:
        """NAF (Non-Adjacent Form) 点乘法"""
        # 转换为NAF表示
        naf = self._to_naf(k)
        
        result = INFINITY
        for i in range(len(naf) - 1, -1, -1):
            result = point_add(result, result, self.p, self.a)
            if naf[i] == 1:
                result = point_add(result, P, self.p, self.a)
            elif naf[i] == -1:
                result = point_add(result, self._negate_point(P), self.p, self.a)
        
        return result
    
    def _to_naf(self, k: int) -> List[int]:
        """将整数转换为NAF表示"""
        naf = []
        k_remaining = k
        
        while k_remaining > 0:
            if k_remaining % 2 == 1:
                remainder = 2 - (k_remaining % 4)
                naf.append(remainder)
                k_remaining -= remainder
            else:
                naf.append(0)
            k_remaining //= 2
        
        return naf
    
    def _window_multiply(self, P: Point, k: int, window_size: int = 4) -> Point:
        """滑动窗口点乘法"""
        # 预计算窗口表
        window_table = [INFINITY]
        for i in range(1, 2 ** window_size):
            window_table.append(point_add(window_table[-1], P, self.p, self.a))
        
        result = INFINITY
        k_bits = bin(k)[2:]  # 转换为二进制字符串
        
        # 处理前导零
        while len(k_bits) % window_size != 0:
            k_bits = '0' + k_bits
        
        # 滑动窗口处理
        for i in range(0, len(k_bits), window_size):
            result = point_add(result, result, self.p, self.a)
            window = k_bits[i:i+window_size]
            if window != '0' * window_size:
                index = int(window, 2)
                result = point_add(result, window_table[index], self.p, self.a)
        
        return result
    
    def _negate_point(self, P: Point) -> Point:
        """点的负值"""
        if P == INFINITY:
            return INFINITY
        return Point(P.x, (-P.y) % self.p)
    
    def sign_optimized(self, message: bytes, private_key: int, 
                       user_id: str = "1234567890123456") -> Tuple[int, int]:
        """优化的SM2签名"""
        start_time = time.time()
        
        # 计算Z值
        public_key = self._fast_point_multiply(self.G, private_key)
        Z = self._compute_z(user_id, public_key)
        
        # 计算e值
        e = self._hash_message(message, Z)
        
        # 使用优化的随机数生成
        k = self._generate_optimized_random()
        
        # 计算签名
        point1 = self._fast_point_multiply(self.G, k)
        x1 = point1.x
        
        r = (e + x1) % self.n
        if r == 0 or r + k == self.n:
            # 重新生成签名
            return self.sign_optimized(message, private_key, user_id)
        
        s = ((1 + private_key) * mod_inverse(k - r * private_key, self.n)) % self.n
        if s == 0:
            return self.sign_optimized(message, private_key, user_id)
        
        end_time = time.time()
        self._last_sign_time = end_time - start_time
        
        return r, s
    
    def _generate_optimized_random(self) -> int:
        """优化的随机数生成"""
        if self.optimization_level == "fast":
            # 使用更快的随机数生成
            return random.getrandbits(256) % self.n
        else:
            return random.randint(1, self.n - 1)
    
    def verify_optimized(self, message: bytes, signature: Tuple[int, int], 
                         public_key: Point, user_id: str = "1234567890123456") -> bool:
        """优化的SM2验证"""
        start_time = time.time()
        
        r, s = signature
        
        # 快速验证
        if not (1 <= r <= self.n - 1 and 1 <= s <= self.n - 1):
            return False
        
        # 计算Z值
        Z = self._compute_z(user_id, public_key)
        e = self._hash_message(message, Z)
        
        t = (r + s) % self.n
        if t == 0:
            return False
        
        # 使用优化的点乘法
        point1 = self._fast_point_multiply(self.G, s)
        point2 = self._fast_point_multiply(public_key, t)
        point3 = point_add(point1, point2, self.p, self.a)
        
        if point3 == INFINITY:
            return False
        
        R = (e + point3.x) % self.n
        
        end_time = time.time()
        self._last_verify_time = end_time - start_time
        
        return R == r
    
    def batch_verify(self, messages: List[bytes], signatures: List[Tuple[int, int]], 
                     public_keys: List[Point], user_ids: List[str] = None) -> List[bool]:
        """批量验证多个签名"""
        if user_ids is None:
            user_ids = ["1234567890123456"] * len(messages)
        
        results = []
        for i in range(len(messages)):
            result = self.verify_optimized(messages[i], signatures[i], 
                                        public_keys[i], user_ids[i])
            results.append(result)
        
        return results
    
    def get_performance_stats(self) -> dict:
        """获取性能统计信息"""
        stats = {
            "optimization_level": self.optimization_level,
            "last_sign_time": getattr(self, '_last_sign_time', 0),
            "last_verify_time": getattr(self, '_last_verify_time', 0),
            "precompute_table_size": len(self._precompute_table)
        }
        return stats

class SM2Parallel(SM2):
    """SM2并行处理版本"""
    
    def __init__(self, num_threads: int = 4):
        super().__init__()
        self.num_threads = num_threads
    
    def parallel_sign(self, messages: List[bytes], private_key: int, 
                     user_id: str = "1234567890123456") -> List[Tuple[int, int]]:
        """并行签名多个消息"""
        import concurrent.futures
        
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.num_threads) as executor:
            futures = []
            for message in messages:
                future = executor.submit(self.sign, message, private_key, user_id)
                futures.append(future)
            
            signatures = []
            for future in concurrent.futures.as_completed(futures):
                signatures.append(future.result())
        
        return signatures
    
    def parallel_verify(self, messages: List[bytes], signatures: List[Tuple[int, int]], 
                       public_keys: List[Point], user_ids: List[str] = None) -> List[bool]:
        """并行验证多个签名"""
        import concurrent.futures
        
        if user_ids is None:
            user_ids = ["1234567890123456"] * len(messages)
        
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.num_threads) as executor:
            futures = []
            for i in range(len(messages)):
                future = executor.submit(self.verify, messages[i], signatures[i], 
                                      public_keys[i], user_ids[i])
                futures.append(future)
            
            results = []
            for future in concurrent.futures.as_completed(futures):
                results.append(future.result())
        
        return results

class SM2MemoryOptimized(SM2):
    """SM2内存优化版本"""
    
    def __init__(self):
        super().__init__()
        self._clear_cache()
    
    def _clear_cache(self):
        """清除缓存"""
        self._point_cache = {}
        self._hash_cache = {}
    
    def sign_memory_optimized(self, message: bytes, private_key: int, 
                             user_id: str = "1234567890123456") -> Tuple[int, int]:
        """内存优化的签名"""
        # 使用缓存避免重复计算
        cache_key = f"{message.hex()}_{private_key}_{user_id}"
        if cache_key in self._hash_cache:
            return self._hash_cache[cache_key]
        
        # 执行签名
        signature = self.sign(message, private_key, user_id)
        
        # 缓存结果
        self._hash_cache[cache_key] = signature
        
        # 限制缓存大小
        if len(self._hash_cache) > 1000:
            self._clear_cache()
        
        return signature
    
    def verify_memory_optimized(self, message: bytes, signature: Tuple[int, int], 
                               public_key: Point, user_id: str = "1234567890123456") -> bool:
        """内存优化的验证"""
        # 使用缓存
        cache_key = f"{message.hex()}_{signature}_{public_key.x}_{public_key.y}_{user_id}"
        if cache_key in self._point_cache:
            return self._point_cache[cache_key]
        
        # 执行验证
        result = self.verify(message, signature, public_key, user_id)
        
        # 缓存结果
        self._point_cache[cache_key] = result
        
        # 限制缓存大小
        if len(self._point_cache) > 1000:
            self._clear_cache()
        
        return result
