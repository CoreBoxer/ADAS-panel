/*
 * Copyright (C) 2014 EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

/**
 * @file CameraToolBox.cpp
 * @brief Implementation of the QML wrapper for OpenCV camera access
 * @author Ayberk Özgür
 * @version 1.0
 * @date 2014-09-22
 */
#include "CameraToolBox.h"

#define WINDOW_WIDTH  960
#define WINDOW_HEIGHT 540

#define VIDEO_SRC "/root/Code/Qt/ADAS/solidYellowLeft.mp4"

CameraToolBox::CameraToolBox(QQuickItem* parent) :
    QQuickItem(parent)
{
    //size = QSize(640,360);
    //size = QSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    size = QSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    //size = QSize(856,480);
    //connect(this, &QQuickItem::parentChanged, this, &CameraToolBox::changeParent);

    lock = new QMutex();
    conImageReady = new QWaitCondition();

    //Open camera right away
    //update();
}

CameraToolBox::~CameraToolBox()
{
    // 因为线程带有while循环，所以使用 Interruption 的机制，先让线程跳出while循环
    // 先关闭图像处理线程，防止conImageReady锁死它
    if(thread_process){
        qDebug() << "~CameraToolBox about to stop thread_process";
        thread_process->requestInterruption();
        thread_process->quit();
        thread_process->wait();
        qDebug() << "thread_process stoped";
    }

    if(thread_capture){
        qDebug() << "~CameraToolBox about to exit thread_capture";
        thread_capture->requestInterruption();
        thread_capture->quit();
        thread_capture->wait();
        qDebug() << "thread_capture stoped";
    }

    delete thread_capture;
    delete camera;
    //Camera release is automatic when cv::VideoCapture is destroyed
}

QSize CameraToolBox::getSize() const
{
    return size;
}

void CameraToolBox::allocateCvImage()
{
    cvImage.release();
    delete[] cvImageBuf;
    cvImageBuf = new unsigned char[size.width()*size.height()*3];
    cvImage = cv::Mat(size.height(),size.width(),CV_8UC3,cvImageBuf);
}

void CameraToolBox::allocateVideoFrame()
{
    videoFrame = new QVideoFrame(size.width()*size.height()*4,size,size.width()*4,VIDEO_OUTPUT_FORMAT);
}

void CameraToolBox::update()
{
    DPRINT("Opening camera %d, width: %d, height: %d", 0, size.width(), size.height());

    //Destroy old thread, camera accessor and buffers
    delete thread_capture;
    delete camera;
    if(videoFrame && videoFrame->isMapped())
        videoFrame->unmap();
    delete videoFrame;
    videoFrame = NULL;
    delete[] cvImageBuf;
    cvImageBuf = NULL;

    //Create new buffers, camera accessor and thread
    if(exportCvImage)
        allocateCvImage();
    if(videoSurface)
        allocateVideoFrame();
    camera = new VideoCaptureWrapper();
    thread_capture = new VideoCaptureThread();
    thread_capture->setParameters(lock, conImageReady,camera, videoFrame, NULL, size.width(),size.height());
    connect(thread_capture,SIGNAL(imageReady()), this, SLOT(imageReceived()));

//    // 注释时要特别注意其他位置的 thread_process 指针，避免野指针崩溃
//    thread_process = new ImageProcessThread();
//    thread_process->setParameters(lock,conImageReady, cvImage, videoFrame, size.width(),size.height());
//    connect(thread_process,SIGNAL(imageReady()), this, SLOT(imageReceived()));


    //Open newly created device
    try{
        //if(camera->open(0)){
        //if(camera->open("D:/OpenCV_output.avi")){
        if(camera->open(VIDEO_SRC)){
        //if(camera->open("D:/road_line.mp4")){
            camera->setProperty(CV_CAP_PROP_FRAME_WIDTH,size.width());
            camera->setProperty(CV_CAP_PROP_FRAME_HEIGHT,size.height());
            if(videoSurface){
                if(videoSurface->isActive())
                    videoSurface->stop();
                if(!videoSurface->start(QVideoSurfaceFormat(size,VIDEO_OUTPUT_FORMAT)))
                    DPRINT("Could not start QAbstractVideoSurface, error: %d",videoSurface->error());
            }
            thread_capture->start();
            //thread_process->start();
            DPRINT("Opened camera %d",0);
        }
        else
            DPRINT("Could not open camera %d",0);
    }
    catch(int e){
        DPRINT("Exception %d",e);
    }
}

void CameraToolBox::imageReceived()
{
    //Update VideoOutput
    if(videoSurface)
        if(!videoSurface->present(*videoFrame))
            DPRINT("Could not present QVideoFrame to QAbstractVideoSurface, error: %d",videoSurface->error());
}

QAbstractVideoSurface* CameraToolBox::getVideoSurface() const
{
    return videoSurface;
}

void CameraToolBox::setVideoSurface(QAbstractVideoSurface* surface)
{
    if(videoSurface != surface){
        videoSurface = surface;
        update();
    }
}

