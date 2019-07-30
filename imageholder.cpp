#include "imageholder.hpp"
#include <QDebug>

static bool isImageFormat(const QString& str)
{
    auto loweredStr = str.toLower();
    return loweredStr.endsWith(".jpg") ||
            loweredStr.endsWith(".jpeg") ||
            loweredStr.endsWith(".bmp") ||
            loweredStr.endsWith(".png");
}

ImageHolder::ImageHolder(QObject *parent)
    : QObject(parent)
{
}

void ImageHolder::LoadImageFile(QString filename)
{
    fileIterator_.LoadFile(filename);
    fileIterator_.FilterFiles(isImageFormat);

    if(!fileIterator_.HasFiles()) {
        emit ImageLoaded(nullptr);
        return;
    }

    LoadImage(filename);
}

void ImageHolder::LoadPreviousImageFile()
{
    if(!fileIterator_.HasFiles()) {
        emit ImageLoaded(nullptr);
        return;
    }

    --fileIterator_;
    LoadImage(*fileIterator_);
}

void ImageHolder::LoadNextImageFile()
{
    if(!fileIterator_.HasFiles()) {
        emit ImageLoaded(nullptr);
        return;
    }

    ++fileIterator_;
    LoadImage(*fileIterator_);
}

void ImageHolder::RevertImage()
{
    if(tmpIMG_) {
        img_ = tmpIMG_;
        emit ImageLoaded(img_);
    }
}

void ImageHolder::StartImageProcessing(std::function<void(QImage&)> processingFunction)
{
    if(img_) {
        tmpIMG_ = std::make_shared<QImage>(*img_);
        processingFunction(*img_);
    }
    emit ProcessingDone(img_);
}

void ImageHolder::StartHistogram()
{
    if(img_) {
        emit HistogramDone(MakeHistogram(*img_));
    }
}

void ImageHolder::LoadImage(const QString& filename)
{
    if(filename.isEmpty()) {
        emit ImageLoaded(nullptr);
        return;
    }

    auto newImg = std::make_unique<QImage>();
    if(!newImg->load(filename)){
        emit ImageLoaded(nullptr);
        return;
    }

    if(QImage::Format::Format_RGB32 != newImg->format()) {
        img_ = std::make_shared<QImage>(newImg->convertToFormat(QImage::Format_RGB32));
    }
    else {
        img_ = std::move(newImg);
    }
    tmpIMG_ = std::make_shared<QImage>(*img_);
    emit ImageLoaded(img_);
}
