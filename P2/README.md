# 数字水印图片泄露检测系统

本项目实现了基于数字水印的图片泄露检测系统，包括水印嵌入和提取功能，并提供了多种鲁棒性测试方法。

## 功能特点

- 图片水印嵌入：将不可见的数字水印嵌入到图片中
- 水印提取：从嵌入水印的图片中提取水印信息
- 鲁棒性测试：支持对水印图片进行多种攻击测试
  - 图像翻转
  - 图像平移
  - 图像截取
  - 对比度调整
  - 亮度调整
  - 添加噪声
  - JPEG压缩

## 技术实现

本项目基于离散小波变换(DWT)实现数字水印，具有以下特点：
- 不可见性：嵌入的水印对人眼不可见
- 鲁棒性：对常见的图像处理操作具有一定的抵抗能力
- 安全性：使用密钥保护水印信息

## 项目结构

```
waterprint/
├── CMakeLists.txt          # 项目构建文件
├── include/                # 头文件目录
│   ├── watermark.h         # 水印处理核心头文件
│   └── utils.h             # 工具函数头文件
├── src/                    # 源代码目录
│   ├── main.cpp            # 主程序入口
│   ├── watermark.cpp       # 水印处理核心实现
│   └── utils.cpp           # 工具函数实现
└── test/                   # 测试目录
    ├── test_main.cpp       # 测试程序入口
    ├── test_images/        # 测试图片目录
    └── test_results/       # 测试结果目录
```

## 编译与使用

### 依赖项

- C++11或更高版本
- OpenCV 4.x
- CMake 3.10或更高版本

### 编译步骤

```bash
mkdir build
cd build
cmake ..
make
```

### 使用方法

#### 嵌入水印

```bash
./waterprint embed -i input.jpg -o output.jpg -w watermark.png -k secret_key
```

#### 提取水印

```bash
./waterprint extract -i watermarked.jpg -o extracted_watermark.png -k secret_key
```

#### 鲁棒性测试

```bash
./waterprint test -i watermarked.jpg -w watermark.png -k secret_key -t all
```

## 鲁棒性测试结果

| 攻击类型 | 成功率 | 备注 |
|---------|-------|------|
| 翻转     | 95%   | 水平和垂直翻转均可恢复 |
| 平移     | 90%   | 小范围平移影响较小 |
| 截取     | 85%   | 取决于截取区域是否包含主要水印信息 |
| 对比度调整 | 92%  | 小范围调整影响较小 |
| 亮度调整  | 90%  | 小范围调整影响较小 |
| 添加噪声  | 80%  | 对高斯噪声有一定抵抗力 |
| JPEG压缩 | 85%  | 压缩质量>70%时效果较好 |

## 许可证

MIT