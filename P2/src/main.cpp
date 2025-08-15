#include "watermark.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

// 函数声明
void embedWatermark(const std::string& inputPath, const std::string& outputPath, 
                   const std::string& watermarkPath, const std::string& key, float alpha);
void extractWatermark(const std::string& inputPath, const std::string& outputPath, 
                     const std::string& key);
void runRobustnessTests(const std::string& watermarkedPath, const std::string& originalWatermarkPath, 
                       const std::string& key, const std::string& testType);
void printTestResult(const std::string& testName, double similarity);

int main(int argc, char** argv) {
    CommandLineParser parser(argc, argv);
    
    // 显示帮助信息
    if (argc < 2 || parser.has("h") || parser.has("help")) {
        parser.printHelp();
        return 0;
    }
    
    std::string command = argv[1];
    
    try {
        if (command == "embed") {
            // 嵌入水印
            std::string inputPath = parser.get("i");
            std::string outputPath = parser.get("o");
            std::string watermarkPath = parser.get("w");
            std::string key = parser.get("k", "default_key");
            float alpha = std::stof(parser.get("a", "0.1"));
            
            if (inputPath.empty() || outputPath.empty() || watermarkPath.empty()) {
                std::cerr << "错误：嵌入水印需要指定输入图像(-i)、输出图像(-o)和水印图像(-w)" << std::endl;
                return 1;
            }
            
            embedWatermark(inputPath, outputPath, watermarkPath, key, alpha);
            
        } else if (command == "extract") {
            // 提取水印
            std::string inputPath = parser.get("i");
            std::string outputPath = parser.get("o");
            std::string key = parser.get("k", "default_key");
            
            if (inputPath.empty() || outputPath.empty()) {
                std::cerr << "错误：提取水印需要指定输入图像(-i)和输出图像(-o)" << std::endl;
                return 1;
            }
            
            extractWatermark(inputPath, outputPath, key);
            
        } else if (command == "test") {
            // 鲁棒性测试
            std::string inputPath = parser.get("i");
            std::string watermarkPath = parser.get("w");
            std::string key = parser.get("k", "default_key");
            std::string testType = parser.get("t", "all");
            
            if (inputPath.empty() || watermarkPath.empty()) {
                std::cerr << "错误：鲁棒性测试需要指定水印图像(-i)和原始水印(-w)" << std::endl;
                return 1;
            }
            
            runRobustnessTests(inputPath, watermarkPath, key, testType);
            
        } else {
            std::cerr << "错误：未知命令 '" << command << "'" << std::endl;
            parser.printHelp();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

void embedWatermark(const std::string& inputPath, const std::string& outputPath, 
                   const std::string& watermarkPath, const std::string& key, float alpha) {
    // 读取原始图像
    cv::Mat original = cv::imread(inputPath);
    if (original.empty()) {
        throw std::runtime_error("无法读取输入图像：" + inputPath);
    }
    
    // 读取水印图像
    cv::Mat watermark = cv::imread(watermarkPath);
    if (watermark.empty()) {
        throw std::runtime_error("无法读取水印图像：" + watermarkPath);
    }
    
    // 创建水印处理对象
    Watermark watermarkProcessor(key);
    
    // 嵌入水印
    std::cout << "正在嵌入水印..." << std::endl;
    cv::Mat watermarked = watermarkProcessor.embed(original, watermark, alpha);
    
    // 保存结果
    bool success = cv::imwrite(outputPath, watermarked);
    if (!success) {
        throw std::runtime_error("无法保存水印图像：" + outputPath);
    }
    
    std::cout << "水印嵌入成功，已保存到：" << outputPath << std::endl;
}

void extractWatermark(const std::string& inputPath, const std::string& outputPath, 
                     const std::string& key) {
    // 读取水印图像
    cv::Mat watermarked = cv::imread(inputPath);
    if (watermarked.empty()) {
        throw std::runtime_error("无法读取水印图像：" + inputPath);
    }
    
    // 创建水印处理对象
    Watermark watermarkProcessor(key);
    
    // 提取水印
    std::cout << "正在提取水印..." << std::endl;
    cv::Mat extractedWatermark = watermarkProcessor.extract(watermarked);
    
    // 保存结果
    bool success = cv::imwrite(outputPath, extractedWatermark);
    if (!success) {
        throw std::runtime_error("无法保存提取的水印：" + outputPath);
    }
    
    std::cout << "水印提取成功，已保存到：" << outputPath << std::endl;
}

void runRobustnessTests(const std::string& watermarkedPath, const std::string& originalWatermarkPath, 
                       const std::string& key, const std::string& testType) {
    // 读取水印图像
    cv::Mat watermarked = cv::imread(watermarkedPath);
    if (watermarked.empty()) {
        throw std::runtime_error("无法读取水印图像：" + watermarkedPath);
    }
    
    // 读取原始水印
    cv::Mat originalWatermark = cv::imread(originalWatermarkPath);
    if (originalWatermark.empty()) {
        throw std::runtime_error("无法读取原始水印：" + originalWatermarkPath);
    }
    
    // 创建水印处理对象
    Watermark watermarkProcessor(key);
    
    std::cout << "开始鲁棒性测试..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试类型\t\t相似度" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // 水平翻转测试
    if (testType == "flip" || testType == "all") {
        cv::Mat flippedH = ImageUtils::flipHorizontal(watermarked);
        cv::Mat extractedH = watermarkProcessor.extract(flippedH);
        double similarityH = watermarkProcessor.calculateSimilarity(originalWatermark, extractedH);
        printTestResult("水平翻转", similarityH);
        
        cv::Mat flippedV = ImageUtils::flipVertical(watermarked);
        cv::Mat extractedV = watermarkProcessor.extract(flippedV);
        double similarityV = watermarkProcessor.calculateSimilarity(originalWatermark, extractedV);
        printTestResult("垂直翻转", similarityV);
    }
    
    // 平移测试
    if (testType == "translate" || testType == "all") {
        cv::Mat translated = ImageUtils::translate(watermarked, 20, 20);
        cv::Mat extracted = watermarkProcessor.extract(translated);
        double similarity = watermarkProcessor.calculateSimilarity(originalWatermark, extracted);
        printTestResult("平移(20,20)", similarity);
    }
    
    // 裁剪测试
    if (testType == "crop" || testType == "all") {
        int width = watermarked.cols;
        int height = watermarked.rows;
        cv::Mat cropped = ImageUtils::crop(watermarked, cv::Rect(width/4, height/4, width/2, height/2));
        cv::Mat extracted = watermarkProcessor.extract(cropped);
        double similarity = watermarkProcessor.calculateSimilarity(originalWatermark, extracted);
        printTestResult("裁剪(50%)", similarity);
    }
    
    // 对比度调整测试
    if (testType == "contrast" || testType == "all") {
        cv::Mat contrastUp = ImageUtils::adjustContrast(watermarked, 1.5);
        cv::Mat extractedUp = watermarkProcessor.extract(contrastUp);
        double similarityUp = watermarkProcessor.calculateSimilarity(originalWatermark, extractedUp);
        printTestResult("增加对比度(1.5x)", similarityUp);
        
        cv::Mat contrastDown = ImageUtils::adjustContrast(watermarked, 0.7);
        cv::Mat extractedDown = watermarkProcessor.extract(contrastDown);
        double similarityDown = watermarkProcessor.calculateSimilarity(originalWatermark, extractedDown);
        printTestResult("降低对比度(0.7x)", similarityDown);
    }
    
    // 亮度调整测试
    if (testType == "brightness" || testType == "all") {
        cv::Mat brightnessUp = ImageUtils::adjustBrightness(watermarked, 30);
        cv::Mat extractedUp = watermarkProcessor.extract(brightnessUp);
        double similarityUp = watermarkProcessor.calculateSimilarity(originalWatermark, extractedUp);
        printTestResult("增加亮度(+30)", similarityUp);
        
        cv::Mat brightnessDown = ImageUtils::adjustBrightness(watermarked, -30);
        cv::Mat extractedDown = watermarkProcessor.extract(brightnessDown);
        double similarityDown = watermarkProcessor.calculateSimilarity(originalWatermark, extractedDown);
        printTestResult("降低亮度(-30)", similarityDown);
    }
    
    // 噪声测试
    if (testType == "noise" || testType == "all") {
        cv::Mat noisy = ImageUtils::addGaussianNoise(watermarked, 0, 15);
        cv::Mat extracted = watermarkProcessor.extract(noisy);
        double similarity = watermarkProcessor.calculateSimilarity(originalWatermark, extracted);
        printTestResult("高斯噪声(σ=15)", similarity);
    }
    
    // JPEG压缩测试
    if (testType == "jpeg" || testType == "all") {
        cv::Mat jpegHigh = ImageUtils::jpegCompression(watermarked, 90);
        cv::Mat extractedHigh = watermarkProcessor.extract(jpegHigh);
        double similarityHigh = watermarkProcessor.calculateSimilarity(originalWatermark, extractedHigh);
        printTestResult("JPEG压缩(90%)", similarityHigh);
        
        cv::Mat jpegLow = ImageUtils::jpegCompression(watermarked, 50);
        cv::Mat extractedLow = watermarkProcessor.extract(jpegLow);
        double similarityLow = watermarkProcessor.calculateSimilarity(originalWatermark, extractedLow);
        printTestResult("JPEG压缩(50%)", similarityLow);
    }
    
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "鲁棒性测试完成" << std::endl;
}

void printTestResult(const std::string& testName, double similarity) {
    std::cout << std::left << std::setw(24) << testName << std::fixed << std::setprecision(4) << similarity * 100 << "%" << std::endl;
}