#ifndef IMAGEPROCESSTHREAD_H
#define IMAGEPROCESSTHREAD_H

#include <QThread>

#include<QDebug>
#include<QMutex>
#include<QWaitCondition>
#include<QVideoFrame>

#include<opencv2/highgui/highgui.hpp>
#include<opencv2/videoio/videoio_c.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/types_c.h>

#include "LaneDetection.h"

using namespace cv;

class ImageProcessThread : public QThread
{
    Q_OBJECT

public:
    explicit ImageProcessThread(QObject *parent = nullptr);
    void setParameters(QMutex* lock, QWaitCondition* conImageReady, cv::Mat srcImage, QVideoFrame* videoFrame, int width, int height);

private:
    QMutex* lock;
    QWaitCondition* conImageReady;
    cv::Mat srcImage;                  ///< Place to export camera image to
    int width;
    int height;

    QVideoFrame* videoFrame;           ///< Place to draw camera image to QML
    LaneDetection* processor;

protected:
    void run() override;

signals:
    void imageReady();
};

#endif // IMAGEPROCESSTHREAD_H
