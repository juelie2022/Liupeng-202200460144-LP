#include "watermark.h"
#include <random>
#include <cmath>

Watermark::Watermark(const std::string& key) : key_(key), dwtLevel_(2), subband_(2) {
    // 默认使用2级DWT分解和HL子带(2)进行水印嵌入
}

void Watermark::setDWTLevel(int level) {
    if (level >= 1 && level <= 3) {
        dwtLevel_ = level;
    }
}

void Watermark::setSubband(int subband) {
    if (subband >= 0 && subband <= 3) {
        subband_ = subband;
    }
}

cv::Mat Watermark::embed(const cv::Mat& original, const cv::Mat& watermark, float alpha) {
    // 检查输入图像
    if (original.empty() || watermark.empty()) {
        throw std::runtime_error("输入图像为空");
    }
    
    // 转换为灰度图像进行处理
    cv::Mat grayOriginal;
    if (original.channels() > 1) {
        cv::cvtColor(original, grayOriginal, cv::COLOR_BGR2GRAY);
    } else {
        grayOriginal = original.clone();
    }
    
    // 调整水印大小，通常水印尺寸应小于原图
    cv::Mat resizedWatermark;
    int watermarkWidth = grayOriginal.cols / 4;
    int watermarkHeight = grayOriginal.rows / 4;
    cv::resize(watermark, resizedWatermark, cv::Size(watermarkWidth, watermarkHeight));
    
    // 将水印转换为二值图像
    cv::Mat binaryWatermark;
    if (resizedWatermark.channels() > 1) {
        cv::cvtColor(resizedWatermark, binaryWatermark, cv::COLOR_BGR2GRAY);
    } else {
        binaryWatermark = resizedWatermark.clone();
    }
    cv::threshold(binaryWatermark, binaryWatermark, 127, 1, cv::THRESH_BINARY);
    
    // 加密水印
    cv::Mat encryptedWatermark = encryptWatermark(binaryWatermark);
    
    // 对原始图像进行DWT变换
    std::vector<cv::Mat> dwtCoeffs = performDWT(grayOriginal, dwtLevel_);
    
    // 选择合适的子带进行水印嵌入
    cv::Mat& targetSubband = dwtCoeffs[subband_];
    
    // 嵌入水印
    for (int y = 0; y < encryptedWatermark.rows; y++) {
        for (int x = 0; x < encryptedWatermark.cols; x++) {
            if (y < targetSubband.rows && x < targetSubband.cols) {
                float watermarkBit = encryptedWatermark.at<uchar>(y, x);
                if (watermarkBit > 0) {
                    targetSubband.at<float>(y, x) += alpha * std::abs(targetSubband.at<float>(y, x));
                } else {
                    targetSubband.at<float>(y, x) -= alpha * std::abs(targetSubband.at<float>(y, x));
                }
            }
        }
    }
    
    // 执行逆DWT变换
    cv::Mat watermarkedGray = performIDWT(dwtCoeffs, dwtLevel_, grayOriginal.size());
    
    // 如果原图是彩色的，则将水印嵌入到亮度通道
    cv::Mat result;
    if (original.channels() > 1) {
        std::vector<cv::Mat> channels;
        cv::Mat originalYCrCb;
        cv::cvtColor(original, originalYCrCb, cv::COLOR_BGR2YCrCb);
        cv::split(originalYCrCb, channels);
        
        // 替换亮度通道
        channels[0] = watermarkedGray;
        
        // 合并通道
        cv::merge(channels, originalYCrCb);
        cv::cvtColor(originalYCrCb, result, cv::COLOR_YCrCb2BGR);
    } else {
        result = watermarkedGray;
    }
    
    return result;
}

cv::Mat Watermark::extract(const cv::Mat& watermarked, const cv::Size& originalSize) {
    // 检查输入图像
    if (watermarked.empty()) {
        throw std::runtime_error("输入图像为空");
    }
    
    // 转换为灰度图像进行处理
    cv::Mat grayWatermarked;
    if (watermarked.channels() > 1) {
        cv::cvtColor(watermarked, grayWatermarked, cv::COLOR_BGR2GRAY);
    } else {
        grayWatermarked = watermarked.clone();
    }
    
    // 对水印图像进行DWT变换
    std::vector<cv::Mat> dwtCoeffs = performDWT(grayWatermarked, dwtLevel_);
    
    // 选择合适的子带进行水印提取
    cv::Mat& targetSubband = dwtCoeffs[subband_];
    
    // 确定水印大小
    cv::Size watermarkSize;
    if (originalSize.width > 0 && originalSize.height > 0) {
        watermarkSize = originalSize;
    } else {
        watermarkSize = cv::Size(targetSubband.cols / 2, targetSubband.rows / 2);
    }
    
    // 提取水印
    cv::Mat extractedWatermark = cv::Mat::zeros(watermarkSize, CV_8UC1);
    
    // 使用阈值方法提取水印
    for (int y = 0; y < extractedWatermark.rows; y++) {
        for (int x = 0; x < extractedWatermark.cols; x++) {
            if (y < targetSubband.rows && x < targetSubband.cols) {
                // 使用局部阈值判断水印位
                float pixelValue = targetSubband.at<float>(y, x);
                extractedWatermark.at<uchar>(y, x) = (pixelValue > 0) ? 1 : 0;
            }
        }
    }
    
    // 解密水印
    cv::Mat decryptedWatermark = decryptWatermark(extractedWatermark);
    
    // 二值化处理，增强对比度
    cv::threshold(decryptedWatermark, decryptedWatermark, 0, 255, cv::THRESH_BINARY);
    
    return decryptedWatermark;
}

double Watermark::calculateSimilarity(const cv::Mat& original, const cv::Mat& extracted) {
    // 确保两个水印大小相同
    cv::Mat resizedOriginal, resizedExtracted;
    cv::Size commonSize(std::min(original.cols, extracted.cols), 
                        std::min(original.rows, extracted.rows));
    
    cv::resize(original, resizedOriginal, commonSize);
    cv::resize(extracted, resizedExtracted, commonSize);
    
    // 转换为二值图像
    cv::Mat binaryOriginal, binaryExtracted;
    cv::threshold(resizedOriginal, binaryOriginal, 127, 1, cv::THRESH_BINARY);
    cv::threshold(resizedExtracted, binaryExtracted, 127, 1, cv::THRESH_BINARY);
    
    // 计算归一化相关系数 (NCC)
    int matchCount = 0;
    int totalPixels = commonSize.width * commonSize.height;
    
    for (int y = 0; y < commonSize.height; y++) {
        for (int x = 0; x < commonSize.width; x++) {
            if (binaryOriginal.at<uchar>(y, x) == binaryExtracted.at<uchar>(y, x)) {
                matchCount++;
            }
        }
    }
    
    return static_cast<double>(matchCount) / totalPixels;
}

std::vector<cv::Mat> Watermark::performDWT(const cv::Mat& image, int level) {
    // 将图像转换为浮点型
    cv::Mat floatImage;
    image.convertTo(floatImage, CV_32F);
    
    // 创建结果向量，存储LL, LH, HL, HH子带
    std::vector<cv::Mat> coeffs(4);
    
    // 创建滤波器
    cv::Mat lowFilter = (cv::Mat_<float>(1, 2) << 0.5, 0.5);
    cv::Mat highFilter = (cv::Mat_<float>(1, 2) << 0.5, -0.5);
    
    cv::Mat tempImage = floatImage.clone();
    cv::Mat LL = tempImage.clone();
    
    // 执行指定级别的DWT
    for (int i = 0; i < level; i++) {
        int rows = LL.rows;
        int cols = LL.cols;
        
        // 创建子带
        cv::Mat L = cv::Mat::zeros(rows, cols / 2, CV_32F);
        cv::Mat H = cv::Mat::zeros(rows, cols / 2, CV_32F);
        cv::Mat LL_new = cv::Mat::zeros(rows / 2, cols / 2, CV_32F);
        cv::Mat LH = cv::Mat::zeros(rows / 2, cols / 2, CV_32F);
        cv::Mat HL = cv::Mat::zeros(rows / 2, cols / 2, CV_32F);
        cv::Mat HH = cv::Mat::zeros(rows / 2, cols / 2, CV_32F);
        
        // 水平方向滤波
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols / 2; c++) {
                float sum_low = 0, sum_high = 0;
                for (int k = 0; k < 2; k++) {
                    int col_idx = 2 * c + k;
                    if (col_idx < cols) {
                        sum_low += LL.at<float>(r, col_idx) * lowFilter.at<float>(0, k);
                        sum_high += LL.at<float>(r, col_idx) * highFilter.at<float>(0, k);
                    }
                }
                L.at<float>(r, c) = sum_low;
                H.at<float>(r, c) = sum_high;
            }
        }
        
        // 垂直方向滤波
        for (int c = 0; c < cols / 2; c++) {
            for (int r = 0; r < rows / 2; r++) {
                float sum_ll = 0, sum_lh = 0, sum_hl = 0, sum_hh = 0;
                for (int k = 0; k < 2; k++) {
                    int row_idx = 2 * r + k;
                    if (row_idx < rows) {
                        sum_ll += L.at<float>(row_idx, c) * lowFilter.at<float>(0, k);
                        sum_lh += L.at<float>(row_idx, c) * highFilter.at<float>(0, k);
                        sum_hl += H.at<float>(row_idx, c) * lowFilter.at<float>(0, k);
                        sum_hh += H.at<float>(row_idx, c) * highFilter.at<float>(0, k);
                    }
                }
                LL_new.at<float>(r, c) = sum_ll;
                LH.at<float>(r, c) = sum_lh;
                HL.at<float>(r, c) = sum_hl;
                HH.at<float>(r, c) = sum_hh;
            }
        }
        
        // 更新LL子带用于下一级分解
        LL = LL_new.clone();
    }
    
    // 存储最终的子带
    coeffs[0] = LL.clone();  // LL子带
    
    // 创建LH, HL, HH子带（简化版本，实际应用中需要更复杂的实现）
    cv::Mat LH = cv::Mat::zeros(LL.size(), CV_32F);
    cv::Mat HL = cv::Mat::zeros(LL.size(), CV_32F);
    cv::Mat HH = cv::Mat::zeros(LL.size(), CV_32F);
    
    // 使用伪随机数填充其他子带（简化实现）
    std::default_random_engine generator(std::hash<std::string>{}(key_));
    std::normal_distribution<float> distribution(0.0, 1.0);
    
    for (int y = 0; y < LL.rows; y++) {
        for (int x = 0; x < LL.cols; x++) {
            LH.at<float>(y, x) = distribution(generator);
            HL.at<float>(y, x) = distribution(generator);
            HH.at<float>(y, x) = distribution(generator);
        }
    }
    
    coeffs[1] = LH;  // LH子带
    coeffs[2] = HL;  // HL子带
    coeffs[3] = HH;  // HH子带
    
    return coeffs;
}

cv::Mat Watermark::performIDWT(const std::vector<cv::Mat>& coeffs, int level, const cv::Size& originalSize) {
    // 简化版本的IDWT实现，实际应用中需要更复杂的实现
    // 这里我们只是简单地将LL子带作为结果返回，并调整大小
    cv::Mat result = coeffs[0].clone();
    cv::resize(result, result, originalSize);
    
    // 归一化到0-255范围
    cv::normalize(result, result, 0, 255, cv::NORM_MINMAX);
    result.convertTo(result, CV_8U);
    
    return result;
}

cv::Mat Watermark::encryptWatermark(const cv::Mat& watermark) {
    // 使用密钥生成伪随机序列
    std::vector<int> randomSequence = generatePseudoRandomSequence(watermark.rows * watermark.cols);
    
    // 创建加密后的水印
    cv::Mat encrypted = watermark.clone();
    
    // 使用异或操作加密水印
    int idx = 0;
    for (int y = 0; y < watermark.rows; y++) {
        for (int x = 0; x < watermark.cols; x++) {
            encrypted.at<uchar>(y, x) = watermark.at<uchar>(y, x) ^ (randomSequence[idx] & 1);
            idx++;
        }
    }
    
    return encrypted;
}

cv::Mat Watermark::decryptWatermark(const cv::Mat& watermark) {
    // 解密过程与加密过程相同（异或操作是可逆的）
    return encryptWatermark(watermark);
}

std::vector<int> Watermark::generatePseudoRandomSequence(int length) {
    // 使用密钥作为种子生成伪随机序列
    std::vector<int> sequence(length);
    std::default_random_engine generator(std::hash<std::string>{}(key_));
    std::uniform_int_distribution<int> distribution(0, 1);
    
    for (int i = 0; i < length; i++) {
        sequence[i] = distribution(generator);
    }
    
    return sequence;
}