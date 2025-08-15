#include "watermark.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>

// 测试函数声明
void testWatermarkEmbedding();
void testWatermarkExtraction();
void testRobustness();
void testPerformance();
void generateTestImage(const std::string& outputPath, int width, int height);
void generateTestWatermark(const std::string& outputPath, int width, int height);

int main(int argc, char** argv) {
    std::cout << "数字水印系统测试程序" << std::endl;
    std::cout << "===================" << std::endl;
    
    // 生成测试图像和水印
    std::string testImagePath = "test/test_images/test_image.png";
    std::string testWatermarkPath = "test/test_images/test_watermark.png";
    
    try {
        // 生成测试图像
        generateTestImage(testImagePath, 512, 512);
        std::cout << "测试图像已生成: " << testImagePath << std::endl;
        
        // 生成测试水印
        generateTestWatermark(testWatermarkPath, 128, 128);
        std::cout << "测试水印已生成: " << testWatermarkPath << std::endl;
        
        // 运行测试
        std::cout << "\n测试1: 水印嵌入" << std::endl;
        testWatermarkEmbedding();
        
        std::cout << "\n测试2: 水印提取" << std::endl;
        testWatermarkExtraction();
        
        std::cout << "\n测试3: 鲁棒性测试" << std::endl;
        testRobustness();
        
        std::cout << "\n测试4: 性能测试" << std::endl;
        testPerformance();
        
        std::cout << "\n所有测试完成" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

void generateTestImage(const std::string& outputPath, int width, int height) {
    // 创建一个测试图像
    cv::Mat testImage(height, width, CV_8UC3);
    
    // 生成渐变背景
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uchar b = static_cast<uchar>(255 * x / width);
            uchar g = static_cast<uchar>(255 * y / height);
            uchar r = static_cast<uchar>(255 * (1 - (x + y) / (width + height)));
            testImage.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
        }
    }
    
    // 添加一些几何图形
    cv::circle(testImage, cv::Point(width/4, height/4), 50, cv::Scalar(0, 0, 255), 2);
    cv::rectangle(testImage, cv::Rect(width/2, height/2, 100, 100), cv::Scalar(0, 255, 0), 2);
    cv::line(testImage, cv::Point(0, 0), cv::Point(width, height), cv::Scalar(255, 0, 0), 2);
    
    // 保存图像
    cv::imwrite(outputPath, testImage);
}

void generateTestWatermark(const std::string& outputPath, int width, int height) {
    // 创建一个测试水印
    cv::Mat watermark(height, width, CV_8UC1, cv::Scalar(0));
    
    // 绘制水印内容（简单的文本和图形）
    cv::putText(watermark, "WATERMARK", cv::Point(10, height/2), 
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255), 2);
    cv::circle(watermark, cv::Point(width/2, height/2), 30, cv::Scalar(255), 2);
    
    // 保存水印
    cv::imwrite(outputPath, watermark);
}

void testWatermarkEmbedding() {
    // 测试水印嵌入功能
    std::string testImagePath = "test/test_images/test_image.png";
    std::string testWatermarkPath = "test/test_images/test_watermark.png";
    std::string outputPath = "test/test_results/watermarked.png";
    
    // 读取测试图像和水印
    cv::Mat original = cv::imread(testImagePath);
    cv::Mat watermark = cv::imread(testWatermarkPath, cv::IMREAD_GRAYSCALE);
    
    if (original.empty() || watermark.empty()) {
        throw std::runtime_error("无法读取测试图像或水印");
    }
    
    // 创建水印处理对象
    Watermark watermarkProcessor("test_key");
    
    // 测试不同强度的水印嵌入
    std::vector<float> alphaValues = {0.05f, 0.1f, 0.2f};
    
    for (float alpha : alphaValues) {
        // 嵌入水印
        cv::Mat watermarked = watermarkProcessor.embed(original, watermark, alpha);
        
        // 保存结果
        std::string alphaStr = std::to_string(alpha);
        alphaStr.erase(alphaStr.find_last_not_of('0') + 1, std::string::npos);
        if (alphaStr.back() == '.') alphaStr.pop_back();
        
        std::string outputPathWithAlpha = outputPath;
        outputPathWithAlpha.insert(outputPathWithAlpha.find_last_of('.'), "_alpha" + alphaStr);
        
        cv::imwrite(outputPathWithAlpha, watermarked);
        std::cout << "  水印强度 " << alpha << " 的结果已保存到: " << outputPathWithAlpha << std::endl;
        
        // 计算PSNR（峰值信噪比）
        double psnr = cv::PSNR(original, watermarked);
        std::cout << "  PSNR: " << std::fixed << std::setprecision(2) << psnr << " dB" << std::endl;
    }
}

void testWatermarkExtraction() {
    // 测试水印提取功能
    std::string watermarkedPath = "test/test_results/watermarked_alpha0.1.png";
    std::string originalWatermarkPath = "test/test_images/test_watermark.png";
    std::string extractedPath = "test/test_results/extracted_watermark.png";
    
    // 读取水印图像和原始水印
    cv::Mat watermarked = cv::imread(watermarkedPath);
    cv::Mat originalWatermark = cv::imread(originalWatermarkPath, cv::IMREAD_GRAYSCALE);
    
    if (watermarked.empty() || originalWatermark.empty()) {
        throw std::runtime_error("无法读取水印图像或原始水印");
    }
    
    // 创建水印处理对象
    Watermark watermarkProcessor("test_key");
    
    // 提取水印
    cv::Mat extractedWatermark = watermarkProcessor.extract(watermarked);
    
    // 保存提取的水印
    cv::imwrite(extractedPath, extractedWatermark);
    std::cout << "  提取的水印已保存到: " << extractedPath << std::endl;
    
    // 计算相似度
    double similarity = watermarkProcessor.calculateSimilarity(originalWatermark, extractedWatermark);
    std::cout << "  与原始水印的相似度: " << std::fixed << std::setprecision(4) << similarity * 100 << "%" << std::endl;
}

void testRobustness() {
    // 测试水印的鲁棒性
    std::string watermarkedPath = "test/test_results/watermarked_alpha0.1.png";
    std::string originalWatermarkPath = "test/test_images/test_watermark.png";
    std::string resultsDir = "test/test_results/";
    
    // 读取水印图像和原始水印
    cv::Mat watermarked = cv::imread(watermarkedPath);
    cv::Mat originalWatermark = cv::imread(originalWatermarkPath, cv::IMREAD_GRAYSCALE);
    
    if (watermarked.empty() || originalWatermark.empty()) {
        throw std::runtime_error("无法读取水印图像或原始水印");
    }
    
    // 创建水印处理对象
    Watermark watermarkProcessor("test_key");
    
    // 测试结果表格头
    std::cout << "  ----------------------------------------" << std::endl;
    std::cout << "  测试类型\t\t相似度" << std::endl;
    std::cout << "  ----------------------------------------" << std::endl;
    
    // 1. 水平翻转测试
    cv::Mat flippedH = ImageUtils::flipHorizontal(watermarked);
    cv::imwrite(resultsDir + "flipped_h.png", flippedH);
    cv::Mat extractedH = watermarkProcessor.extract(flippedH);
    cv::imwrite(resultsDir + "extracted_flipped_h.png", extractedH);
    double similarityH = watermarkProcessor.calculateSimilarity(originalWatermark, extractedH);
    std::cout << "  " << std::left << std::setw(24) << "水平翻转" << std::fixed << std::setprecision(4) << similarityH * 100 << "%" << std::endl;
    
    // 2. 垂直翻转测试
    cv::Mat flippedV = ImageUtils::flipVertical(watermarked);
    cv::imwrite(resultsDir + "flipped_v.png", flippedV);
    cv::Mat extractedV = watermarkProcessor.extract(flippedV);
    cv::imwrite(resultsDir + "extracted_flipped_v.png", extractedV);
    double similarityV = watermarkProcessor.calculateSimilarity(originalWatermark, extractedV);
    std::cout << "  " << std::left << std::setw(24) << "垂直翻转" << std::fixed << std::setprecision(4) << similarityV * 100 << "%" << std::endl;
    
    // 3. 平移测试
    cv::Mat translated = ImageUtils::translate(watermarked, 20, 20);
    cv::imwrite(resultsDir + "translated.png", translated);
    cv::Mat extractedT = watermarkProcessor.extract(translated);
    cv::imwrite(resultsDir + "extracted_translated.png", extractedT);
    double similarityT = watermarkProcessor.calculateSimilarity(originalWatermark, extractedT);
    std::cout << "  " << std::left << std::setw(24) << "平移(20,20)" << std::fixed << std::setprecision(4) << similarityT * 100 << "%" << std::endl;
    
    // 4. 裁剪测试
    int width = watermarked.cols;
    int height = watermarked.rows;
    cv::Mat cropped = ImageUtils::crop(watermarked, cv::Rect(width/4, height/4, width/2, height/2));
    cv::imwrite(resultsDir + "cropped.png", cropped);
    cv::Mat extractedC = watermarkProcessor.extract(cropped);
    cv::imwrite(resultsDir + "extracted_cropped.png", extractedC);
    double similarityC = watermarkProcessor.calculateSimilarity(originalWatermark, extractedC);
    std::cout << "  " << std::left << std::setw(24) << "裁剪(50%)" << std::fixed << std::setprecision(4) << similarityC * 100 << "%" << std::endl;
    
    // 5. 对比度调整测试
    cv::Mat contrastUp = ImageUtils::adjustContrast(watermarked, 1.5);
    cv::imwrite(resultsDir + "contrast_up.png", contrastUp);
    cv::Mat extractedCU = watermarkProcessor.extract(contrastUp);
    cv::imwrite(resultsDir + "extracted_contrast_up.png", extractedCU);
    double similarityCU = watermarkProcessor.calculateSimilarity(originalWatermark, extractedCU);
    std::cout << "  " << std::left << std::setw(24) << "增加对比度(1.5x)" << std::fixed << std::setprecision(4) << similarityCU * 100 << "%" << std::endl;
    
    // 6. 亮度调整测试
    cv::Mat brightnessUp = ImageUtils::adjustBrightness(watermarked, 30);
    cv::imwrite(resultsDir + "brightness_up.png", brightnessUp);
    cv::Mat extractedBU = watermarkProcessor.extract(brightnessUp);
    cv::imwrite(resultsDir + "extracted_brightness_up.png", extractedBU);
    double similarityBU = watermarkProcessor.calculateSimilarity(originalWatermark, extractedBU);
    std::cout << "  " << std::left << std::setw(24) << "增加亮度(+30)" << std::fixed << std::setprecision(4) << similarityBU * 100 << "%" << std::endl;
    
    // 7. 噪声测试
    cv::Mat noisy = ImageUtils::addGaussianNoise(watermarked, 0, 15);
    cv::imwrite(resultsDir + "noisy.png", noisy);
    cv::Mat extractedN = watermarkProcessor.extract(noisy);
    cv::imwrite(resultsDir + "extracted_noisy.png", extractedN);
    double similarityN = watermarkProcessor.calculateSimilarity(originalWatermark, extractedN);
    std::cout << "  " << std::left << std::setw(24) << "高斯噪声(σ=15)" << std::fixed << std::setprecision(4) << similarityN * 100 << "%" << std::endl;
    
    // 8. JPEG压缩测试
    cv::Mat jpegLow = ImageUtils::jpegCompression(watermarked, 50);
    cv::imwrite(resultsDir + "jpeg_low.png", jpegLow);
    cv::Mat extractedJL = watermarkProcessor.extract(jpegLow);
    cv::imwrite(resultsDir + "extracted_jpeg_low.png", extractedJL);
    double similarityJL = watermarkProcessor.calculateSimilarity(originalWatermark, extractedJL);
    std::cout << "  " << std::left << std::setw(24) << "JPEG压缩(50%)" << std::fixed << std::setprecision(4) << similarityJL * 100 << "%" << std::endl;
    
    std::cout << "  ----------------------------------------" << std::endl;
}

void testPerformance() {
    // 测试水印嵌入和提取的性能
    std::string testImagePath = "test/test_images/test_image.png";
    std::string testWatermarkPath = "test/test_images/test_watermark.png";
    
    // 读取测试图像和水印
    cv::Mat original = cv::imread(testImagePath);
    cv::Mat watermark = cv::imread(testWatermarkPath, cv::IMREAD_GRAYSCALE);
    
    if (original.empty() || watermark.empty()) {
        throw std::runtime_error("无法读取测试图像或水印");
    }
    
    // 创建水印处理对象
    Watermark watermarkProcessor("test_key");
    
    // 测试嵌入性能
    std::cout << "  测试嵌入性能..." << std::endl;
    auto startEmbed = std::chrono::high_resolution_clock::now();
    
    // 执行多次嵌入以获得更准确的性能测量
    const int numIterations = 10;
    cv::Mat watermarked;
    
    for (int i = 0; i < numIterations; i++) {
        watermarked = watermarkProcessor.embed(original, watermark, 0.1f);
    }
    
    auto endEmbed = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> embedTime = endEmbed - startEmbed;
    double avgEmbedTime = embedTime.count() / numIterations;
    
    std::cout << "  平均嵌入时间: " << std::fixed << std::setprecision(4) << avgEmbedTime << " 秒" << std::endl;
    
    // 测试提取性能
    std::cout << "  测试提取性能..." << std::endl;
    auto startExtract = std::chrono::high_resolution_clock::now();
    
    cv::Mat extracted;
    for (int i = 0; i < numIterations; i++) {
        extracted = watermarkProcessor.extract(watermarked);
    }
    
    auto endExtract = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> extractTime = endExtract - startExtract;
    double avgExtractTime = extractTime.count() / numIterations;
    
    std::cout << "  平均提取时间: " << std::fixed << std::setprecision(4) << avgExtractTime << " 秒" << std::endl;
}