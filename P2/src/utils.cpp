#include "utils.h"
#include <random>
#include <vector>

// ImageUtils 类实现

cv::Mat ImageUtils::flipHorizontal(const cv::Mat& image) {
    cv::Mat result;
    cv::flip(image, result, 1); // 1表示水平翻转
    return result;
}

cv::Mat ImageUtils::flipVertical(const cv::Mat& image) {
    cv::Mat result;
    cv::flip(image, result, 0); // 0表示垂直翻转
    return result;
}

cv::Mat ImageUtils::translate(const cv::Mat& image, int dx, int dy) {
    cv::Mat result = cv::Mat::zeros(image.size(), image.type());
    cv::Mat translationMatrix = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);
    cv::warpAffine(image, result, translationMatrix, image.size());
    return result;
}

cv::Mat ImageUtils::crop(const cv::Mat& image, const cv::Rect& rect) {
    // 确保裁剪区域在图像范围内
    cv::Rect safeRect = rect & cv::Rect(0, 0, image.cols, image.rows);
    return image(safeRect).clone();
}

cv::Mat ImageUtils::adjustContrast(const cv::Mat& image, double factor) {
    cv::Mat result;
    image.convertTo(result, -1, factor, 0);
    return result;
}

cv::Mat ImageUtils::adjustBrightness(const cv::Mat& image, int delta) {
    cv::Mat result;
    image.convertTo(result, -1, 1, delta);
    return result;
}

cv::Mat ImageUtils::addGaussianNoise(const cv::Mat& image, double mean, double stddev) {
    cv::Mat noise = cv::Mat(image.size(), image.type());
    cv::randn(noise, mean, stddev);
    cv::Mat result;
    cv::add(image, noise, result);
    return result;
}

cv::Mat ImageUtils::jpegCompression(const cv::Mat& image, int quality) {
    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, quality};
    cv::imencode(".jpg", image, buffer, params);
    return cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
}

// CommandLineParser 类实现

CommandLineParser::CommandLineParser(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.substr(0, 1) == "-") {
            std::string option = arg.substr(1);
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options_[option] = argv[i + 1];
                i++;
            } else {
                options_[option] = "true";
            }
        }
    }
}

std::string CommandLineParser::get(const std::string& option, const std::string& defaultValue) {
    if (options_.find(option) != options_.end()) {
        return options_[option];
    }
    return defaultValue;
}

bool CommandLineParser::has(const std::string& option) {
    return options_.find(option) != options_.end();
}

void CommandLineParser::printHelp() {
    std::cout << "数字水印图片泄露检测系统使用说明：" << std::endl;
    std::cout << "嵌入水印：" << std::endl;
    std::cout << "  ./waterprint embed -i <输入图像> -o <输出图像> -w <水印图像> [-k <密钥>] [-a <强度>]" << std::endl;
    std::cout << "提取水印：" << std::endl;
    std::cout << "  ./waterprint extract -i <水印图像> -o <提取水印输出> [-k <密钥>]" << std::endl;
    std::cout << "鲁棒性测试：" << std::endl;
    std::cout << "  ./waterprint test -i <水印图像> -w <原始水印> [-k <密钥>] [-t <测试类型>]" << std::endl;
    std::cout << std::endl;
    std::cout << "参数说明：" << std::endl;
    std::cout << "  -i: 输入图像路径" << std::endl;
    std::cout << "  -o: 输出图像路径" << std::endl;
    std::cout << "  -w: 水印图像路径" << std::endl;
    std::cout << "  -k: 加密密钥（可选，默认为'default_key'）" << std::endl;
    std::cout << "  -a: 水印强度（可选，范围0.0-1.0，默认为0.1）" << std::endl;
    std::cout << "  -t: 测试类型（可选，可选值：flip, translate, crop, contrast, brightness, noise, jpeg, all）" << std::endl;
    std::cout << "  -h: 显示帮助信息" << std::endl;
}