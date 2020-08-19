#include "ImageProcessThread.h"

ImageProcessThread::ImageProcessThread(QObject *parent)
    :QThread(parent)
{
    this->processor = new LaneDetection();
}

void ImageProcessThread::setParameters(QMutex *lock, QWaitCondition* conImageReady, cv::Mat srcImage, QVideoFrame* videoFrame, int width, int height)
{
    this->lock = lock;
    this->conImageReady = conImageReady;
    this->srcImage = srcImage;
    this->width = width;
    this->height = height;
    this->videoFrame = videoFrame;
}

void ImageProcessThread::run()
{
    // run into when 'videoFrame' is provided by parent.
    cv::Mat screenImage;
    if(videoFrame != NULL){
        //Assuming desktop, RGB camera image and RGBA QVideoFrame
        // 将 videoFrame 的二进制内容映射到cv::Mat型结构中，用来做一个cvtColor格式转换
        videoFrame->map(QAbstractVideoBuffer::ReadOnly);
        screenImage = cv::Mat(height,width,CV_8UC4,videoFrame->bits());
    }

    while(!isInterruptionRequested())
    {
        lock->lock();
        conImageReady->wait(lock);// 相当重要，FPS从4提高到18

        /******************** Image Process Code BEGIN ********************/

        Mat dstImage;
        dstImage = processor->process(srcImage);
//        cv::imshow("out", dstImage);
//        cv::waitKey(1);


//        Mat gray, edge, edge_output;

//        cvtColor(srcImage, gray, COLOR_RGB2GRAY);

//        blur(gray, gray, Size(9, 9), Point(-1,-1), BORDER_DEFAULT);

//        Canny(gray, edge, 50, 50 * 2, 3, false);

//        cvtColor(gray, edge_output, COLOR_GRAY2RGB);


//        cv::imshow("srcImage", srcImage);
//        cv::imshow("screenImage", screenImage);
//        cv::waitKey(1);

//        cv::Mat dstImage;
//        srcImage.copyTo(dstImage);
//        cv::circle(srcImage, cv::Point(srcImage.cols/ 2, srcImage.rows / 2), 30, cv::Scalar(0, 0, 255));
//        cv::cvtColor(srcImage, dstImage, cv::COLOR_RGB2GRAY);




        /******************** Image Process Code END ********************/
        /****************** careful about the format ********************/
//        cv::imshow("rece", dstImage);
//        cv::waitKey(1);


        // run into when 'videoFrame' is provided by parent.
        //Get camera image into screen frame buffer
        if(videoFrame != NULL){
            //cv::Mat tempMat(height,width,CV_8UC3,dstImage.data);
            cv::cvtColor(dstImage, screenImage, cv::COLOR_RGB2RGBA);
            // emit only when 'videoFrame' is provided by parent.
            emit imageReady();
        }

        lock->unlock();
    }
}
