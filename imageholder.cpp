#include "imageholder.hpp"
#include <QDebug>

static bool isImageFormat(const QString& str)
{
    const auto loweredStr = str.toLower();
    return loweredStr.endsWith(".jpg") ||
            loweredStr.endsWith(".jpeg") ||
            loweredStr.endsWith(".bmp") ||
            loweredStr.endsWith(".png");
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

void ImageHolder::StartImageProcessing(std::function<void(QImage&)> processingFunction)
{
    auto pImage = imageStates_.CurrentState();
    if(pImage) {
        auto tmpImage = std::make_shared<QImage>(*pImage); // deep copy
        processingFunction(*tmpImage);
        imageStates_.AddState(tmpImage);
    }
    emit ProcessingDone(imageStates_.CurrentState());
}

void ImageHolder::StartHistogram()
{
    auto pImage = imageStates_.CurrentState();
    if(pImage) {
        auto tmpImage = std::make_shared<QImage>(*pImage); // deep copy
        emit HistogramDone(MakeHistogram(*tmpImage));
    }
    else {
        emit HistogramDone(HistRGB());
    }
}

void ImageHolder::LoadImage(const QString& filename)
{
    auto newImg = std::make_unique<QImage>();
    if(!newImg->load(filename)){
        emit ImageLoaded(nullptr);
        return;
    }

    imageStates_.Clear();
    imageStates_.AddState(std::move(newImg));
    emit ImageLoaded(imageStates_.CurrentState());
}
