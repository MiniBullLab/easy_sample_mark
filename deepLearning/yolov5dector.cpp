#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "yolov5dector.h"
#include <iostream>
#include <iomanip>

#define IDX(o) (entry_index(o,anchor,j,i,grid_x,grid_y))

static int g_classificationCnt = 1;

static float sigmoid_x(float x)
{
    return static_cast<float>(1.f / (1.f + exp(-x)));
}

static int entry_index(int loc, int anchorC, int w, int h, int lWidth, int lHeight)
{
    return ((anchorC *(3+5) + loc) * lHeight * lWidth + h * lWidth + w);
}

static float overlap(float x1, float w1, float x2, float w2)
{
    float left = std::max(x1 - w1 / 2.0f, x2 - w2 / 2.0f);
    float right = std::min(x1 + w1 / 2.0f, x2 + w2 / 2.0f);
    return right - left;
}

static float cal_iou(std::vector<float> box, std::vector<float>truth)
{
    float w = overlap(box[0], box[2], truth[0], truth[2]);
    float h = overlap(box[1], box[3], truth[1], truth[3]);
    if(w < 0 || h < 0)
        return 0;

    float inter_area = w * h;
    float union_area = box[2] * box[3] + truth[2] * truth[3] - inter_area;
    return inter_area * 1.0f / union_area;
}

YoloV5Dector::YoloV5Dector()
{
    initData();
}

YoloV5Dector::~YoloV5Dector()
{

}

int YoloV5Dector::initModel(const std::string modelNet, const std::string modelWeight)
{
    try
    {
        net = cv::dnn::readNet(modelNet);
    }
    catch (const cv::Exception &err) //Importer can throw errors, we will catch them
    {
        std::cout << err.msg << std::endl;
        return -1;
    }
    return 0;
}

int YoloV5Dector::initDetectorParameters(const int dataWidth, const int dataHeight,
                                     const float confidenceThreshold, const QMap<int, QString> &labels)
{
    this->inputDataWidth = dataWidth;
    this->inputDataHeight = dataHeight;
    this->confidenceThreshold = confidenceThreshold;
    this->labelIds.clear();
    this->labelNames.clear();
    for(QMap<int, QString>::const_iterator iterator = labels.constBegin(); iterator != labels.constEnd(); ++iterator)
    {
        this->labelIds.push_back(iterator.key());
        this->labelNames.push_back(iterator.value().toStdString());
    }
    g_classificationCnt = this->labelIds.size();
    // std::cout << g_classificationCnt << confidenceThreshold << dataWidth << dataHeight <<  std::endl;
    return 0;
}

void YoloV5Dector::processDetect(const cv::Mat &inputImage, std::vector<Detect2dBox> &objectRect)
{
    objectRect.clear();
    if(inputImage.empty())
    {
        std::cout <<"input image empty!" << std::endl;
        return;
    }
    cv::Mat blob;
    cv::Size srcSize(inputImage.cols, inputImage.rows);
    double duration = static_cast<double>(cv::getTickCount());
    cv::dnn::blobFromImage(inputImage.clone(), blob, 1 / 255.0,
                           cv::Size(this->inputDataWidth, this->inputDataHeight),
                           cv::Scalar(0, 0, 0), true, false);
    this->net.setInput(blob);
    std::vector<cv::Mat> netOutputImg;
    this->net.forward(netOutputImg, net.getUnconnectedOutLayersNames());
    postprocess(srcSize, netOutputImg, objectRect);
    std::cout <<"objectRect:" << objectRect.size() << std::endl;
    duration = (static_cast<double>(cv::getTickCount()) - duration) / cv::getTickFrequency();
    std::cout << "time(sec):" << std::fixed << std::setprecision(4) << duration << std::endl;
    //    showDetection(const_cast<cv::Mat&>(image), objectRect);
    //    cv::imshow("yolov5", image);
    //    cv::waitKey(20);
}

int YoloV5Dector::postprocess(const cv::Size &srcSize, const std::vector<cv::Mat> &outputs, std::vector<Detect2dBox> &det_results)
{
    int rval = 0;

    std::vector<int> classIds;//结果id数组
    std::vector<float> confidences;//结果每个id对应置信度数组
    // std::vector<cv::Rect> boxes;//每个id矩形框
    std::vector<std::vector<float>> boxes;
    float ratio_h = (float)srcSize.height / this->inputDataHeight;
    float ratio_w = (float)srcSize.width / this->inputDataWidth;

    for (int stride = 0; stride < 3; stride++)
    {    //stride
        std::vector<float> box;
        float* pdata = (float*)outputs[stride].data;
        int grid_x = (int)(this->inputDataWidth / this->stride[stride]);
        int grid_y = (int)(this->inputDataHeight / this->stride[stride]);
        float max_class_socre;
        int cls = 0;
        for (int anchor = 0; anchor < 3; anchor++) { //anchors
            const float anchor_w = this->anchors[stride][anchor * 2];
            const float anchor_h = this->anchors[stride][anchor * 2 + 1];
            for (int i = 0; i < grid_y; i++) {
                for (int j = 0; j < grid_x; j++) {
                    float box_score = sigmoid_x(pdata[IDX(4)]);//获取每一行的box框中含有某个物体的概率
                    if (box_score > this->objThreshold) {
                        // cv::Mat scores(1, this->classes.size(), CV_32FC1, pdata + 5);
                        // Point classIdPoint;
                        // double max_class_socre;
                        // minMaxLoc(scores, 0, &max_class_socre, 0, &classIdPoint);
                        max_class_socre = -1e10;
                        cls = 0;
                        for (int p = 0; p < g_classificationCnt; p++) {
                            if (pdata[IDX(5 + p)] >= max_class_socre) {
                                max_class_socre = pdata[IDX(5 + p)];
                                cls = p;
                            }
                        }

                        max_class_socre = sigmoid_x(max_class_socre);
                        box_score *= max_class_socre;

                        // max_class_socre = sigmoid_x((float)max_class_socre); // (float)max_class_socre;
                        if (box_score > this->confidenceThreshold) {
                            // rect [x,y,w,h]
                            box.clear();
                            float x = (sigmoid_x(pdata[IDX(0)]) * 2.f - 0.5f + j) * this->stride[stride]; //x pdata[0];   //
                            float y = (sigmoid_x(pdata[IDX(1)]) * 2.f - 0.5f + i) * this->stride[stride]; //y pdata[1]; //
                            float w = powf(sigmoid_x(pdata[IDX(2)]) * 2.f, 2.f) * anchor_w; //w pdata[2]; //
                            float h = powf(sigmoid_x(pdata[IDX(3)]) * 2.f, 2.f) * anchor_h; //h pdata[3]; //
                            int left = (x - 0.5*w)*ratio_w;
                            int top = (y - 0.5*h)*ratio_h;
                            classIds.push_back(cls);
                            confidences.push_back(box_score);
                            box.push_back(float(left));
                            box.push_back(float(top));
                            box.push_back(w*ratio_w);
                            box.push_back(h*ratio_h);
                            box.push_back(float(cls));
                            box.push_back(float(box_score));
                            boxes.push_back(box);
                        }
                    }
                }
            }
        }
    }

    boxes = applyNMS(boxes, this->nmsThreshold);
    for (int i = 0; i < boxes.size(); i++) {
        Detect2dBox result;
        std::string objectName = getLabelName(boxes[i][4]);
        result.objectName = objectName;
        result.classID = boxes[i][4];
        // if (result.classID == 0)
        // {
        result.confidence = boxes[i][5];
        result.minX = boxes[i][0];
        // if (result.x1 < 1) {continue;}
        result.minY = boxes[i][1];
        // if (result.y1 < 1) {continue;}
        result.maxX = boxes[i][0] + boxes[i][2];
        // if (result.x2 >= this->src_width) {result.x2 = this->src_width - 1;}
        result.maxY = boxes[i][1] + boxes[i][3];
        // if (result.y2 >= 1080) {result.y2 = 1080 - 1;}
        // std::cout << result.minX << " " << result.minY << " " << result.maxX << " " << result.maxY << std::endl;
        det_results.push_back(result);
        // }
    }

    return rval;
}

std::vector<std::vector<float>> YoloV5Dector::applyNMS(std::vector<std::vector<float>>& boxes, const float thres)
{
    std::vector<std::vector<float>> result;
    std::vector<bool> exist_box(boxes.size(), true);

    int n = 0;
    for (size_t _i = 0; _i < boxes.size(); ++_i)
    {
        if (!exist_box[_i])
            continue;
        n = 0;
        for (size_t _j = _i + 1; _j < boxes.size(); ++_j)
        {
            // different class name
            if (!exist_box[_j] || boxes[_i][4] != boxes[_j][4])
                continue;
            float ovr = cal_iou(boxes[_j], boxes[_i]);
            if (ovr >= thres)
            {
                if (boxes[_j][5] <= boxes[_i][5])
                {
                    exist_box[_j] = false;
                }
                else
                {
                    n++;   // have object_score bigger than boxes[_i]
                    exist_box[_i] = false;
                    break;
                }
            }
        }
        //if (n) exist_box[_i] = false;
        if (n == 0)
        {
            result.push_back(boxes[_i]);
        }
    }

    return result;
}

void YoloV5Dector::initData()
{
    this->inputDataWidth = 576;
    this->inputDataHeight = 352;
    this->confidenceThreshold = 0.3f;

    this->labelIds.clear();
    this->labelNames.clear();

    this->anchors = {{10.0, 13.0, 16.0, 30.0, 33.0, 23.0}, \
                     {30.0, 61.0, 62.0, 45.0, 59.0, 119.0}, \
                     {116.0, 90.0, 156.0, 198.0, 373.0, 326.0}};
    this->stride = {8.0f, 16.0f, 32.0f};
    this->nmsThreshold = 0.45f;
    this->objThreshold = 0.3f;
}

void YoloV5Dector::saveConfig()
{

}

void YoloV5Dector::loadConfig()
{

}

