#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <string>

/**
 * @brief 图像处理工具类，提供各种图像处理和测试功能
 */
class ImageUtils {
public:
    /**
     * @brief 对图像进行水平翻转
     * @param image 输入图像
     * @return 翻转后的图像
     */
    static cv::Mat flipHorizontal(const cv::Mat& image);
    
    /**
     * @brief 对图像进行垂直翻转
     * @param image 输入图像
     * @return 翻转后的图像
     */
    static cv::Mat flipVertical(const cv::Mat& image);
    
    /**
     * @brief 对图像进行平移
     * @param image 输入图像
     * @param dx x方向平移量（正值向右，负值向左）
     * @param dy y方向平移量（正值向下，负值向上）
     * @return 平移后的图像
     */
    static cv::Mat translate(const cv::Mat& image, int dx, int dy);
    
    /**
     * @brief 对图像进行裁剪
     * @param image 输入图像
     * @param rect 裁剪区域
     * @return 裁剪后的图像
     */
    static cv::Mat crop(const cv::Mat& image, const cv::Rect& rect);
    
    /**
     * @brief 调整图像对比度
     * @param image 输入图像
     * @param factor 对比度因子 (>1增加对比度，<1降低对比度)
     * @return 调整后的图像
     */
    static cv::Mat adjustContrast(const cv::Mat& image, double factor);
    
    /**
     * @brief 调整图像亮度
     * @param image 输入图像
     * @param delta 亮度增量 (正值增加亮度，负值降低亮度)
     * @return 调整后的图像
     */
    static cv::Mat adjustBrightness(const cv::Mat& image, int delta);
    
    /**
     * @brief 添加高斯噪声
     * @param image 输入图像
     * @param mean 噪声均值
     * @param stddev 噪声标准差
     * @return 添加噪声后的图像
     */
    static cv::Mat addGaussianNoise(const cv::Mat& image, double mean, double stddev);
    
    /**
     * @brief JPEG压缩模拟
     * @param image 输入图像
     * @param quality JPEG质量 (0-100)
     * @return 压缩后的图像
     */
    static cv::Mat jpegCompression(const cv::Mat& image, int quality);
};

/**
 * @brief 命令行参数解析类
 */
class CommandLineParser {
public:
    /**
     * @brief 构造函数
     * @param argc 参数数量
     * @param argv 参数数组
     */
    CommandLineParser(int argc, char** argv);
    
    /**
     * @brief 获取命令行参数值
     * @param option 选项名
     * @param defaultValue 默认值
     * @return 参数值
     */
    std::string get(const std::string& option, const std::string& defaultValue = "");
    
    /**
     * @brief 检查选项是否存在
     * @param option 选项名
     * @return 是否存在
     */
    bool has(const std::string& option);
    
    /**
     * @brief 打印帮助信息
     */
    void printHelp();

private:
    std::map<std::string, std::string> options_;
};

#endif // UTILS_H