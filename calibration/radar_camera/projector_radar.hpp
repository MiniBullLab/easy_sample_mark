#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include "birdview.hpp"
#include "baseAlgorithm/common_transform.h"

struct Pt {
  cv::Point point;
  float dist;
  float x;
};

class Projector {
public:
  cv::Mat oriCloud;
  std::vector<cv::Point3f> point3DList;
  int point_size_ = 6;
  Eigen::Matrix3d homograph_;
  BirdView bv_handler;

  void init(const cv::Mat &img, const cv::Mat &homograph,
            const int &bv_w, const int &bv_h,
            std::vector<cv::Point2f> &picked_points) {
    homograph_(0, 0) = homograph.at<float>(0, 0);
    homograph_(0, 1) = homograph.at<float>(0, 1);
    homograph_(0, 2) = homograph.at<float>(0, 2);
    homograph_(1, 0) = homograph.at<float>(1, 0);
    homograph_(1, 1) = homograph.at<float>(1, 1);
    homograph_(1, 2) = homograph.at<float>(1, 2);
    homograph_(2, 0) = homograph.at<float>(2, 0);
    homograph_(2, 1) = homograph.at<float>(2, 1);
    homograph_(2, 2) = homograph.at<float>(2, 2);
    bv_handler.init(img, bv_w, bv_h, homograph_, picked_points);
  }

  void setPointSize(int size) { point_size_ = size; }

  bool loadPointCloud(const std::vector<cv::Point3f> &radarList) {
      point3DList = radarList;
      oriCloud = cv::Mat(cv::Size(radarList.size(), 3), CV_32FC1);
      for (size_t i = 0; i < radarList.size(); ++i) {
          oriCloud.at<float>(0, i) = radarList[i].x;
          oriCloud.at<float>(1, i) = radarList[i].y;
          oriCloud.at<float>(2, i) = 0;
      }
      return true;
  }

  cv::Scalar fakeColor(float value) {
    float posSlope = 255 / 60.0;
    float negSlope = -255 / 60.0;
    value *= 255;
    cv::Vec3f color;
    if (value < 60) {
      color[0] = 255;
      color[1] = posSlope * value + 0;
      color[2] = 0;
    } else if (value < 120) {
      color[0] = negSlope * value + 2 * 255;
      color[1] = 255;
      color[2] = 0;
    } else if (value < 180) {
      color[0] = 0;
      color[1] = 255;
      color[2] = posSlope * value - 2 * 255;
    } else if (value < 240) {
      color[0] = 0;
      color[1] = negSlope * value + 4 * 255;
      color[2] = 255;
    } else if (value < 300) {
      color[0] = posSlope * value - 4 * 255;
      color[1] = 0;
      color[2] = 255;
    } else {
      color[0] = 255;
      color[1] = 0;
      color[2] = negSlope * value + 6 * 255;
    }
    return cv::Scalar(color[0], color[1], color[2]);
  }

  void ProjectImage(const cv::Mat &img, const cv::Mat &K, const cv::Mat &D,
               const Transform &transform, cv::Mat &current_frame) {
      cv::Mat outImg = img.clone();
      std::vector<cv::Point2f> tempResult;
      std::vector<cv::Point2f> projectPoints;
      cv::Mat rotationVector; // Rotation vector
      cv::Mat translationVector; // Translation vector
      if(point3DList.size() <= 0)
      {
          return;
      }
      Transform::VectorRotation vectorRotation = transform.vectorRation();
      Transform::Translation T = transform.translation();
      rotationVector = (cv::Mat_<float>(1, 3) << vectorRotation[0], vectorRotation[1], vectorRotation[2]);
      std::cout << rotationVector << std::endl;
      translationVector = (cv::Mat_<float>(1, 3) << T[0], T[1], T[2]);
      cv::projectPoints(point3DList, rotationVector, translationVector, K, D, tempResult);

      for(const cv::Point2f& point: tempResult)
      {
          if((point.x >= 0 && point.x < img.cols)
                  && (point.y >= 0 && point.y < img.rows))
          {
              projectPoints.push_back(point);
          }
      }
      for(const auto &point: projectPoints)
      {
          cv::circle(outImg, cv::Point(point.x, point.y), point_size_, cv::Scalar(0, 0, 255), -1);
      }
      current_frame = outImg;
  }

  void ProjectBirdView(const Transform &transform, cv::Mat &bv_frame) {
      if(point3DList.size() <= 0)
      {
          return;
      }
      std::vector<cv::Point3f> bv_pts;
      for(size_t i = 0; i < point3DList.size(); i++)
      {
          Eigen::Vector3f tempPoint(point3DList[i].x, point3DList[i].y, point3DList[i].z);
          Eigen::Affine3f tempTransform(transform.matrix());
          Eigen::Vector3f ransformPoint = tempTransform * tempPoint;
          bv_pts.push_back(cv::Point3f(ransformPoint(0), ransformPoint(1), 1));
      }
      bv_frame = bv_handler.drawProjectBirdView(point_size_, bv_pts);
  }

  void ProjectToRawMat(const cv::Mat &img, const cv::Mat &K, const cv::Mat &D,
                       const Eigen::Matrix4f &matrix,
                       cv::Mat &current_frame, cv::Mat &bv_frame) {
      cv::Mat I = cv::Mat::eye(3, 3, CV_32FC1);
      cv::Mat mapX, mapY;
      cv::Mat outImg = cv::Mat(img.size(), CV_32FC3);
      cv::initUndistortRectifyMap(K, D, I, K, img.size(), CV_32FC1, mapX, mapY);
      cv::remap(img, outImg, mapX, mapY, cv::INTER_LINEAR);
      cv::Mat dist = oriCloud.rowRange(0, 1).mul(oriCloud.rowRange(0, 1)) +
              oriCloud.rowRange(1, 2).mul(oriCloud.rowRange(1, 2)) +
              oriCloud.rowRange(2, 3).mul(oriCloud.rowRange(2, 3));
      cv::Mat R_ = (cv::Mat_<float>(3, 3) << matrix(0, 0), matrix(0, 1), matrix(0, 2),
                                             matrix(1, 0), matrix(1, 1), matrix(1, 2),
                                             matrix(2, 0), matrix(1, 2), matrix(2, 2));
      cv::Mat T_ = (cv::Mat_<float>(3, 1) << matrix(0, 3) , matrix(1, 3) , matrix(2, 3));

      // cv::Mat projCloud2d =
      //     K * (R_ * oriCloud + repeat(T_, 1, oriCloud.cols));
      cv::Mat transCloud2d = R_ * oriCloud + repeat(T_, 1, oriCloud.cols);
      float maxDist = 0;
      float maxx = 0;
      std::vector<Pt> points;
      std::vector<cv::Point3f> bv_radar_pts;
      for (int32_t i = 0; i < transCloud2d.cols; ++i) {
          float x = transCloud2d.at<float>(0, i);
          float y = transCloud2d.at<float>(1, i);
          float z = transCloud2d.at<float>(2, i);

          Eigen::Vector3d world_pt(x, z, 1);
          // x, z, 1???
          Eigen::Vector3d bv_pt = homograph_.inverse().eval() * world_pt;
          bv_pt /= bv_pt(2);
          // cv::Point result(cvRound(bv_pt(0)), cvRound(bv_pt(1)));
          int x2d = cvRound(bv_pt(0));
          int y2d = cvRound(bv_pt(1));
          float d = sqrt(dist.at<float>(0, i));
          if (x2d >= 0 && y2d >= 0 && x2d < img.cols && y2d < img.rows && z > 0) {
              maxx = std::max(maxx, std::fabs(x));
              maxDist = std::max(maxDist, d);
              points.push_back(Pt{cv::Point(x2d, y2d), d, std::fabs(x)});
              bv_radar_pts.push_back(cv::Point3f(x, z, 1));
          }
      }

      bv_frame = bv_handler.drawProjectBirdView(point_size_, bv_radar_pts);

      sort(points.begin(), points.end(),
           [](const Pt &a, const Pt &b) { return a.dist > b.dist; });
      for (size_t i = 0; i < points.size(); ++i) {
          cv::Scalar color;
          // distance
          float d = points[i].dist;
          float x = points[i].x;
          // color = fakeColor(d / maxDist);
          color = fakeColor(x / maxx);
          circle(outImg, points[i].point, point_size_, color, -1);
      }
      current_frame = outImg;
  }

  cv::Mat ProjectToFisheyeMat(cv::Mat img, cv::Mat K, cv::Mat D, cv::Mat R,
                              cv::Mat T) {
    cv::Mat I = cv::Mat::eye(3, 3, CV_32FC1);
    cv::Mat mapX, mapY;
    cv::Mat outImg = cv::Mat(img.size(), CV_32FC3);
    // if broader view needed, change the new_K and new_Size
    cv::fisheye::initUndistortRectifyMap(K, D, I, K, img.size(), CV_32FC1, mapX,
                                         mapY);
    cv::remap(img, outImg, mapX, mapY, cv::INTER_LINEAR);
    cv::Mat dist = oriCloud.rowRange(0, 1).mul(oriCloud.rowRange(0, 1)) +
                   oriCloud.rowRange(1, 2).mul(oriCloud.rowRange(1, 2)) +
                   oriCloud.rowRange(2, 3).mul(oriCloud.rowRange(2, 3));
    cv::Mat R_ = R;
    cv::Mat T_ = T;

    cv::Mat projCloud2d = K * (R_ * oriCloud + repeat(T_, 1, oriCloud.cols));
    float maxDist = 0;

    std::vector<Pt> points;
    for (int32_t i = 0; i < projCloud2d.cols; ++i) {
      float x = projCloud2d.at<float>(0, i);
      float y = projCloud2d.at<float>(1, i);
      float z = projCloud2d.at<float>(2, i);
      int32_t x2d = cvRound(x / z);
      int32_t y2d = cvRound(y / z);
      float d = sqrt(dist.at<float>(0, i));
      if (x2d >= 0 && y2d >= 0 && x2d < img.cols && y2d < img.rows && z > 0) {
        maxDist = std::max(maxDist, d);
        points.push_back(Pt{cv::Point(x2d, y2d), d});
      }
    }
    sort(points.begin(), points.end(),
         [](const Pt &a, const Pt &b) { return a.dist > b.dist; });
    for (int32_t i = 0; i < static_cast<int32_t>(points.size()); ++i) {
      cv::Scalar color;
      // distance
      float d = points[i].dist;
      color = fakeColor(d / maxDist);
      circle(outImg, points[i].point, point_size_, color, -1);
    }
    return outImg;
  }

  cv::Mat ProjectToFisheyeImage(std::string imgName, cv::Mat K, cv::Mat D,
                                cv::Mat R, cv::Mat T) {
    cv::Mat img = cv::imread(imgName);
    return ProjectToFisheyeMat(img, K, D, R, T);
  }
};
