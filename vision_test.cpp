#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    //1. 读取图像
    cv::Mat img = cv::imread("armor.jpg");
    if (img.empty()) {
        std::cerr << "Error: Could not open or find the image!" << std::endl;
        return -1;
    }
    //2, 由 BGR 转换为 HSV 颜色空间
    cv::Mat hsv;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);
    //3.过滤光晕背景
    //s(饱和度)下限设为50，v(亮度)的下限都提高到 180，过滤掉暗红色的光晕
    cv::Mat mask1, mask2, mask;
    // 提取红色，把 V (亮度) 的下限从 50 提高到 120，过滤掉暗红色的光晕
    cv::inRange(hsv, cv::Scalar(0, 150, 180), cv::Scalar(10, 255, 255), mask1);
    cv::inRange(hsv, cv::Scalar(160, 150, 180), cv::Scalar(180, 255, 255), mask2);
    // 合并两个白色区域
    cv::bitwise_or(mask1, mask2, mask);

    // 形态学操作：用稍微大一点的核，比如 5x5，把光晕削掉
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // 找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    // 复制原图用于绘制结果
    cv::Mat result_img = img.clone();
    
    // 遍历所有轮廓
    for (size_t i = 0; i < contours.size(); i++) {
        // 获取该轮廓的最小外接矩形
        cv::Rect rect = cv::boundingRect(contours[i]);
        // 计算长宽比和面积
        float aspect_ratio = (float)rect.width / rect.height;
        float area = cv::contourArea(contours[i]);

        // 1. 面积过滤：太小的是噪点（<100），太大的是背景光晕（>10000）
        // 2. 长宽比过滤：灯条一定是细长的（宽/高 通常小于 1）
        if (area > 300 && area < 50000 && aspect_ratio < 0.6) { 
            // 画框
            cv::rectangle(result_img, rect, cv::Scalar(0, 255, 0), 2);
            std::cout << "找到灯条! 面积: " << area << " 宽高比: " << aspect_ratio << std::endl;
        }
    }
    // 保存结果图像
    cv::imwrite("red_light_contours.jpg", result_img);
    std::cout << "Success! Check red_light_contours.jpg" << std::endl;

    return 0;
}