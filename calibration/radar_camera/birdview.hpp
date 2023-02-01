#pragma once

#include <Eigen/Dense>
#include <algorithm>
#include <iostream>
#include <memory>
#include <opencv2/core/eigen.hpp>
#include <opencv2/opencv.hpp>
#include <string>

#include <vector>

class BirdView {
public:
  Eigen::Matrix3d ground2img_hmat_;
  Eigen::Matrix3d img2ground_hmat_;
  cv::Mat img2bv_hmat_;
  cv::Mat ground2bv_hmat_;
  int width_;
  int height_;

  cv::Mat image_;
  cv::Mat const_image_;

  cv::Size origin_img_size_;
  cv::Mat grid_image_;

  bool computeHomo(const std::vector<cv::Point2f> &picked_points) {
    std::vector<cv::Point2f> ground_pts;
    std::vector<cv::Point2f> img_pts;
    std::vector<cv::Point2f> bv_pts;
    for (int i = 0; i < 4; i++) {
      Eigen::Vector3d ground_pt =
          img2ground_hmat_ *
          Eigen::Vector3d(picked_points[i].x, picked_points[i].y, 1);
      ground_pt /= ground_pt(2);
      ground_pts.push_back(cv::Point2f(ground_pt(0), ground_pt(1)));
    }
    float left = width_ / 2.8;
    float right = width_ - width_ / 2.8;
    float up = height_ / 2.5;
    float down = height_ - height_ / 25.0;
    bv_pts.push_back(cv::Point2f(left, down));
    bv_pts.push_back(cv::Point2f(left, up));
    bv_pts.push_back(cv::Point2f(right, down));
    bv_pts.push_back(cv::Point2f(right, up));

    ground2bv_hmat_ = cv::findHomography(ground_pts, bv_pts);
    grid_image_ = cv::Mat(cv::Size(width_, height_),  CV_8UC3, cv::Scalar(235, 235, 235));
    // draw lines
    int gap = (width_ + 10.0) / 10.0;
    for (int i = 0; i < 10; i++) {
      cv::Point start = cv::Point(gap * i, 0);
      cv::Point end = cv::Point(gap * i, height_);
      cv::line(grid_image_, start, end, cv::Scalar(140, 140, 140), 2);
    }
    gap = (height_ + 10.0) / 15.0;
    for (int i = 0; i < 15; i++) {
      cv::Point start = cv::Point(0, gap * i);
      cv::Point end = cv::Point(width_, gap * i);
      cv::line(grid_image_, start, end, cv::Scalar(140, 140, 140), 2);
    }

    // draw lane lines
    cv::line(grid_image_, cv::Point(int(left), 0),
             cv::Point(int(left), height_), cv::Scalar(225, 80, 0), 4);
    cv::line(grid_image_, cv::Point(int(right), 0),
             cv::Point(int(right), height_), cv::Scalar(225, 80, 0), 4);
    return true;
  }

  void init(const cv::Mat &img, const int &width, const int &height,
            const Eigen::Matrix3d &img2ground_hmat,
            const std::vector<cv::Point2f> &picked_points) {
    origin_img_size_ = img.size();
    img.copyTo(image_);
    image_.copyTo(const_image_);

    width_ = width;
    height_ = height;
    img2ground_hmat_ = img2ground_hmat;
    computeHomo(picked_points);
  }

  cv::Mat drawProjectBirdView(const int &point_size,
                              const std::vector<cv::Point2f> &grount_pts) {
    cv::Mat output;
    output = grid_image_.clone();

    Eigen::Matrix3d hmat;
    cv::cv2eigen(ground2bv_hmat_, hmat);
    for (size_t i = 0; i < grount_pts.size(); i++) {
      Eigen::Vector3d pt(grount_pts[i].x * 0.2, grount_pts[i].y * 0.2, 1.0);
      Eigen::Vector3d bv_pt = hmat * pt;
      bv_pt /= bv_pt(2);
      int xbv = cvRound(bv_pt(0));
      int ybv = cvRound(bv_pt(1));
      std::cout << "x: " << xbv << " "
                << "y: " << ybv << std::endl;
      cv::Scalar color = cv::Scalar(0, 0, 255);
      cv::circle(output, cv::Point(xbv, ybv), point_size, color, -1);
    }
    return output;
  }
};
