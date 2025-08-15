"""
SM2核心功能测试
"""

import pytest
import hashlib
from sm2.core import SM2
from sm2.utils import Point, INFINITY

class TestSM2Core:
    """SM2核心功能测试类"""
    
    def setup_method(self):
        """测试前准备"""
        self.sm2 = SM2()
        self.test_message = b"Hello, SM2!"
        self.test_user_id = "1234567890123456"
    
    def test_sm2_initialization(self):
        """测试SM2初始化"""
        assert self.sm2.p > 0
        assert self.sm2.a >= 0
        assert self.sm2.b >= 0
        assert self.sm2.n > 0
        assert self.sm2.G != INFINITY
    
    def test_keypair_generation(self):
        """测试密钥对生成"""
        public_key, private_key = self.sm2.generate_keypair()
        
        assert isinstance(public_key, Point)
        assert isinstance(private_key, int)
        assert private_key > 0
        assert private_key < self.sm2.n
        assert public_key != INFINITY
    
    def test_signature_generation(self):
        """测试签名生成"""
        public_key, private_key = self.sm2.generate_keypair()
        signature = self.sm2.sign(self.test_message, private_key, self.test_user_id)
        
        assert isinstance(signature, tuple)
        assert len(signature) == 2
        r, s = signature
        assert isinstance(r, int)
        assert isinstance(s, int)
        assert 1 <= r <= self.sm2.n - 1
        assert 1 <= s <= self.sm2.n - 1
    
    def test_signature_verification(self):
        """测试签名验证"""
        public_key, private_key = self.sm2.generate_keypair()
        signature = self.sm2.sign(self.test_message, private_key, self.test_user_id)
        
        # 验证有效签名
        is_valid = self.sm2.verify(self.test_message, signature, public_key, self.test_user_id)
        assert is_valid == True
        
        # 验证无效消息
        invalid_message = b"Invalid message"
        is_valid = self.sm2.verify(invalid_message, signature, public_key, self.test_user_id)
        assert is_valid == False
    
    def test_encryption_decryption(self):
        """测试加密解密"""
        public_key, private_key = self.sm2.generate_keypair()
        original_message = b"Secret message for encryption"
        
        # 加密
        ciphertext = self.sm2.encrypt(original_message, public_key)
        assert isinstance(ciphertext, bytes)
        assert len(ciphertext) > len(original_message)
        
        # 解密
        decrypted_message = self.sm2.decrypt(ciphertext, private_key)
        assert decrypted_message == original_message
    
    def test_z_value_computation(self):
        """测试Z值计算"""
        public_key, _ = self.sm2.generate_keypair()
        Z = self.sm2._compute_z(self.test_user_id, public_key)
        
        assert isinstance(Z, bytes)
        assert len(Z) == 32  # SHA-256输出长度
    
    def test_message_hashing(self):
        """测试消息哈希"""
        public_key, _ = self.sm2.generate_keypair()
        Z = self.sm2._compute_z(self.test_user_id, public_key)
        e = self.sm2._hash_message(self.test_message, Z)
        
        assert isinstance(e, int)
        assert e >= 0
    
    def test_invalid_signature_verification(self):
        """测试无效签名验证"""
        public_key, _ = self.sm2.generate_keypair()
        
        # 无效的r值
        invalid_signature1 = (0, 12345)
        is_valid = self.sm2.verify(self.test_message, invalid_signature1, public_key, self.test_user_id)
        assert is_valid == False
        
        # 无效的s值
        invalid_signature2 = (12345, 0)
        is_valid = self.sm2.verify(self.test_message, invalid_signature2, public_key, self.test_user_id)
        assert is_valid == False
        
        # r + s = n
        invalid_signature3 = (self.sm2.n - 1, 1)
        is_valid = self.sm2.verify(self.test_message, invalid_signature3, public_key, self.test_user_id)
        assert is_valid == False
    
    def test_curve_point_validation(self):
        """测试曲线点验证"""
        # 有效点
        valid_point = Point(12345, 67890)
        is_valid = self.sm2._is_point_on_curve(valid_point)
        # 注意：这个点可能不在曲线上，所以结果可能是False
        
        # 无穷远点
        is_valid = self.sm2._is_point_on_curve(INFINITY)
        assert is_valid == True
    
    def test_kdf_function(self):
        """测试密钥派生函数"""
        point = Point(12345, 67890)
        length = 16
        
        result = self.sm2._kdf(point, length)
        assert isinstance(result, bytes)
        assert len(result) == length
    
    def test_multiple_signatures(self):
        """测试多个签名"""
        public_key, private_key = self.sm2.generate_keypair()
        messages = [b"Message 1", b"Message 2", b"Message 3"]
        
        signatures = []
        for message in messages:
            signature = self.sm2.sign(message, private_key, self.test_user_id)
            signatures.append(signature)
        
        # 验证所有签名
        for i, message in enumerate(messages):
            is_valid = self.sm2.verify(message, signatures[i], public_key, self.test_user_id)
            assert is_valid == True
    
    def test_different_user_ids(self):
        """测试不同用户ID"""
        public_key, private_key = self.sm2.generate_keypair()
        user_ids = ["User1", "User2", "User3"]
        
        for user_id in user_ids:
            signature = self.sm2.sign(self.test_message, private_key, user_id)
            is_valid = self.sm2.verify(self.test_message, signature, public_key, user_id)
            assert is_valid == True
    
    def test_large_message(self):
        """测试大消息"""
        public_key, private_key = self.sm2.generate_keypair()
        large_message = b"X" * 1000  # 1000字节的消息
        
        signature = self.sm2.sign(large_message, private_key, self.test_user_id)
        is_valid = self.sm2.verify(large_message, signature, public_key, self.test_user_id)
        assert is_valid == True
    
    def test_empty_message(self):
        """测试空消息"""
        public_key, private_key = self.sm2.generate_keypair()
        empty_message = b""
        
        signature = self.sm2.sign(empty_message, private_key, self.test_user_id)
        is_valid = self.sm2.verify(empty_message, signature, public_key, self.test_user_id)
        assert is_valid == True

if __name__ == "__main__":
    pytest.main([__file__])
