#ifndef CAMERATOOLBOX_H
#define CAMERATOOLBOX_H

#include <QQuickItem>
#include <QCameraInfo>
#include <QList>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QSize>
#include <QMutex>
#include <QWaitCondition>
#include <QVariant>

#include <opencv2/highgui/highgui.hpp>

#include "VideoCaptureThread.h"
#include "ImageProcessThread.h"


/**
 * @brief Let QMetaType know the type provided,
 *        then QVariant need it to customize it's type.
 */
Q_DECLARE_METATYPE(cv::Mat)

/**
 * @brief QML wrapper for OpenCV camera access
 */
class CameraToolBox : public QQuickItem {
Q_OBJECT
    Q_DISABLE_COPY(CameraToolBox)
    Q_PROPERTY(QAbstractVideoSurface* videoSurface READ getVideoSurface WRITE setVideoSurface)
    //Q_PROPERTY(QSize size READ getSize WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QSize size READ getSize NOTIFY sizeChanged)

public:

    /**
     * @brief Creates a new cv camera with the given QML parent
     *
     * @param parent The QML parent
     */
    CameraToolBox(QQuickItem* parent = 0);

    /**
     * @brief Destroys this cv camera
     */
    ~CameraToolBox();

    /**
     * @brief Gets the video surface associated with this camera
     *
     * @return The video surface associated with this camera
     */
    QAbstractVideoSurface* getVideoSurface() const;

    /**
     * @brief Gives a video surface for this camera to draw on
     *
     * @param surface The new surface to draw on
     */
    void setVideoSurface(QAbstractVideoSurface* videoSurface);
    QSize getSize() const;

    QMutex* lock;
    QWaitCondition* conImageReady;

private:

    const QVideoFrame::PixelFormat VIDEO_OUTPUT_FORMAT = QVideoFrame::PixelFormat::Format_ARGB32;

    QSize size;                                             ///< The desired camera resolution
    VideoCaptureWrapper* camera = NULL;                     ///< The camera object
    VideoCaptureThread* thread_capture = NULL;              ///< Thread to run camera image acquisition
    ImageProcessThread* thread_process = NULL;              ///< Thread to process captured image

    QVideoFrame* videoFrame = NULL;                         ///< Object that contains the image buffer to be presented to the video surface
    QAbstractVideoSurface* videoSurface = NULL;             ///< Object that presents the videoFrame to the outside world

    bool exportCvImage = true;                              ///< Whether to export the CV image
    cv::Mat cvImage;                                        ///< Buffer to export the camera image to
    unsigned char* cvImageBuf = NULL;                       ///< Actual physical buffer for cv::Mat, it is helpless without it

    /**
     * @brief Closes and opens the desired camera with possibly new parameters
     */
    void update();

    /**
     * @brief Allocates an appropriately sized cv::Mat depending on the output size
     */
    void allocateCvImage();

    /**
     * @brief Allocates an appropriately sized and formatted QVideoFrame
     */
    void allocateVideoFrame();


signals:
    /**
     * @brief Emitted after the camera image size has changed
     */
    void sizeChanged();

private slots:

    /**
     * @brief Callback for when image from the camera is received
     */
    void imageReceived();
};

#endif // CAMERATOOLBOX_H
