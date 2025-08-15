#ifndef WATERMARK_H
#define WATERMARK_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

/**
 * @brief 水印处理类，提供水印嵌入和提取功能
 */
class Watermark {
public:
    /**
     * @brief 构造函数
     * @param key 用于水印加密的密钥
     */
    Watermark(const std::string& key = "default_key");

    /**
     * @brief 在图像中嵌入水印
     * @param original 原始图像
     * @param watermark 水印图像（应为二值图像）
     * @param alpha 水印强度参数 (0.0-1.0)
     * @return 嵌入水印后的图像
     */
    cv::Mat embed(const cv::Mat& original, const cv::Mat& watermark, float alpha = 0.1);

    /**
     * @brief 从图像中提取水印
     * @param watermarked 嵌入水印的图像
     * @param originalSize 原始水印的大小（可选）
     * @return 提取的水印图像
     */
    cv::Mat extract(const cv::Mat& watermarked, const cv::Size& originalSize = cv::Size(0, 0));

    /**
     * @brief 计算两个水印的相似度
     * @param original 原始水印
     * @param extracted 提取的水印
     * @return 相似度 (0.0-1.0)
     */
    double calculateSimilarity(const cv::Mat& original, const cv::Mat& extracted);

    /**
     * @brief 设置水印嵌入的DWT级别
     * @param level DWT分解级别 (1-3)
     */
    void setDWTLevel(int level);

    /**
     * @brief 设置水印嵌入的子带
     * @param subband 子带选择 (0: LL, 1: LH, 2: HL, 3: HH)
     */
    void setSubband(int subband);

private:
    std::string key_; // 加密密钥
    int dwtLevel_;    // DWT分解级别
    int subband_;     // 子带选择

    // DWT变换函数
    std::vector<cv::Mat> performDWT(const cv::Mat& image, int level);
    
    // IDWT逆变换函数
    cv::Mat performIDWT(const std::vector<cv::Mat>& coeffs, int level, const cv::Size& originalSize);
    
    // 水印加密函数
    cv::Mat encryptWatermark(const cv::Mat& watermark);
    
    // 水印解密函数
    cv::Mat decryptWatermark(const cv::Mat& watermark);
    
    // 生成伪随机序列
    std::vector<int> generatePseudoRandomSequence(int length);
};

#endif // WATERMARK_H