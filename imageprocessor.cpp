#include "imageprocessor.hpp"

#include "imageprocessing.hpp"

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{}

void ImageProcessor::MedianFilterGo(const QImage& img, const int ksz)
{
    auto pImage = std::make_shared<QImage>(img);
    MedianFilter(*pImage, ksz);
    emit isDone(pImage);
}

void ImageProcessor::CustomFilterGo(const QImage& img, const std::vector<double> &kernel)
{
    auto pImage = std::make_shared<QImage>(img);
    CustomFilter(*pImage, kernel);
    emit isDone(pImage);
}

void ImageProcessor::ErosionGo(const QImage& img, int ksz)
{
    auto pImage = std::make_shared<QImage>(img);
    Erosion(*pImage, ksz);
    emit isDone(pImage);
}

void ImageProcessor::IncreaseGo(const QImage& img, int ksz)
{
    auto pImage = std::make_shared<QImage>(img);
    Increase(*pImage, ksz);
    emit isDone(pImage);
}

void ImageProcessor::RotateLeftGo(const QImage& img)
{
    auto pImage = std::make_shared<QImage>(img);
    RotateLeft(*pImage);
    emit isDone(pImage);
}

void ImageProcessor::RotateRightGo(const QImage& img)
{
    auto pImage = std::make_shared<QImage>(img);
    RotateRight(*pImage);
    emit isDone(pImage);
}

void ImageProcessor::HMirrorGo(const QImage& img)
{
    auto pImage = std::make_shared<QImage>(img.mirrored(false, true));
    emit isDone(pImage);
}

void ImageProcessor::VMirrorGo(const QImage& img)
{
    auto pImage = std::make_shared<QImage>(img.mirrored(true, false));
    emit isDone(pImage);
}

void ImageProcessor::GrayWorldGo(const QImage &img)
{
    auto pImage = std::make_shared<QImage>(img);
    GrayWorld(*pImage);
    emit isDone(pImage);
}

void ImageProcessor::LinearCorrGo(const QImage &img)
{
    auto pImage = std::make_shared<QImage>(img);
    LinearCorrection(*pImage);
    emit isDone(pImage);
}

void ImageProcessor::GammaFuncGo(const QImage& img, double c, double d)
{
    auto pImage = std::make_shared<QImage>(img);
    GammaFunc(*pImage, c, d);
    emit isDone(pImage);
}

void ImageProcessor::GaussBlurGo(const QImage &img)
{
    auto pImage = std::make_shared<QImage>(img);
    GaussBlur(*pImage);
    emit isDone(pImage);
}
