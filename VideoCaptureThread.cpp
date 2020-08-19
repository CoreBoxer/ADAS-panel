#include "VideoCaptureThread.h"

#define PROCESS_IN_CAPTURE_THREAD

VideoCaptureThread::VideoCaptureThread(QObject *parent)
    :QThread(parent)
{
    this->processor = new LaneDetection();
}

VideoCaptureThread::~VideoCaptureThread()
{

}

void VideoCaptureThread::setParameters(QMutex* lock, QWaitCondition* conImageReady, VideoCaptureWrapper *camera, QVideoFrame *videoFrame, unsigned char *cvImageBuf, int width, int height)
{
    this->running = true;
    this->conImageReady = conImageReady;
    this->lock = lock;
    this->camera = camera;
    this->videoFrame = videoFrame;
    this->cvImageBuf = cvImageBuf;
    this->width = width;
    this->height = height;
}

void VideoCaptureThread::run()
{
#if defined(QT_DEBUG)
    QElapsedTimer timer;
    float fps = 0.0f;
    int millisElapsed = 0;
    int millis;
    timer.start();
#endif

    // run into when 'videoFrame' is provided by parent.
    cv::Mat screenImage;
    if(videoFrame != NULL){
        //Assuming desktop, RGB camera image and RGBA QVideoFrame
        // 将 videoFrame 的二进制内容映射到cv::Mat型结构中，用来做一个cvtColor格式转换
        videoFrame->map(QAbstractVideoBuffer::ReadOnly);
        screenImage = cv::Mat(height,width,CV_8UC4,videoFrame->bits());
    }


    //while(!isInterruptionRequested() && running && videoFrame != NULL && camera != NULL){
    while(!isInterruptionRequested() && running && camera != NULL){

        lock->lock();

        if(!camera->grabFrame())
            break;

        unsigned char* cameraFrame = camera->retrieveFrame();
#ifdef PROCESS_IN_CAPTURE_THREAD
        Mat dstImage;
        cv::Mat tempMat(height,width,CV_8UC3,cameraFrame);
        dstImage = processor->process(tempMat);

        // run into when 'videoFrame' is provided by parent.
        //Get camera image into screen frame buffer
        if(videoFrame != NULL){
            cv::cvtColor(dstImage,screenImage,cv::COLOR_RGB2RGBA);
            // emit only when 'videoFrame' is provided by parent.
            emit imageReady();
        }
#else
        // run into when 'videoFrame' is provided by parent.
        //Get camera image into screen frame buffer
        if(videoFrame != NULL){
            cv::Mat tempMat(height,width,CV_8UC3,cameraFrame);
            cv::cvtColor(tempMat,screenImage,cv::COLOR_RGB2RGBA);
            // emit only when 'videoFrame' is provided by parent.
            emit imageReady();
        }

        //Export camera image
        if(cvImageBuf){
        //Assuming desktop, RGB camera image
            memcpy(cvImageBuf,cameraFrame,height*width*3);
        }
#endif
        //QThread::msleep(33);

        // 解锁图像处理线程
        conImageReady->wakeAll();
        lock->unlock();


#if defined(QT_DEBUG)
        millis = (int)timer.restart();
        millisElapsed += millis;
        fps = CAM_FPS_RATE*fps + (1.0f - CAM_FPS_RATE)*(1000.0f/millis);
        if(millisElapsed >= CAM_FPS_PRINT_PERIOD){
            qDebug("Camera is running at %f FPS",fps);
            millisElapsed = 0;
        }
#endif
    }
    exec();
}

