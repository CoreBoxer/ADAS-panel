#ifndef VIDEOCAPTURETHREAD_H
#define VIDEOCAPTURETHREAD_H

#include<QDebug>
#include<QThread>
#include<QObject>
#include<QElapsedTimer>
#include<QVideoFrame>
#include <QMutex>
#include <QWaitCondition>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/videoio/videoio_c.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/types_c.h>

#include<vector>

#include"VideoCaptureWrapper.h"
#include "LaneDetection.h"

using namespace cv;

#ifdef QT_DEBUG
#define DPRINT(...) qDebug(__VA_ARGS__)
#else
#define DPRINT(...) while(0);
#endif

class VideoCaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoCaptureThread(QObject *parent = nullptr);
    ~VideoCaptureThread();
    void setParameters(QMutex* lock, QWaitCondition* conImageReady, VideoCaptureWrapper* camera, QVideoFrame* videoFrame, unsigned char* cvImageBuf, int width, int height);

private:

#if defined(QT_DEBUG) && !defined(ANDROID) //Android camera has its own FPS debug info
    const float CAM_FPS_RATE = 0.9f;            ///< Rate of using the older FPS estimates
    const int CAM_FPS_PRINT_PERIOD = 500;       ///< Period of printing the FPS estimate, in milliseconds
#endif

    QMutex* lock;
    QWaitCondition* conImageReady;
    int width;                                  ///< Width of the camera image
    int height;                                 ///< Height of the camera image
    VideoCaptureWrapper* camera;                 ///< The camera to get data from
    bool running = false;                       ///< Whether the worker thread is running
    QVideoFrame* videoFrame;                    ///< Place to draw camera image to
    unsigned char* cvImageBuf;                  ///< Place to export camera image to
    LaneDetection* processor;

protected:
    void run() override;

signals:
    void imageReady();
};

#endif // VIDEOCAPTURETHREAD_H
