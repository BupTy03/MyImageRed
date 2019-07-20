#include "imageprocessor.hpp"

#include "imageprocessing.hpp"

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{}

void ImageProcessor::MedianFilterGo(QImage *img, const int ksz)
{
    assert(img != nullptr);
    MedianFilter(*img, ksz);
    emit isDone();
}

void ImageProcessor::CustomFilterGo(QImage* img, const std::vector<double>* kernel)
{
    assert(img != nullptr);
    assert(kernel != nullptr);
    CustomFilter(*img, *kernel);
    emit isDone();
}

void ImageProcessor::ErosionGo(QImage *img, int ksz)
{
    assert(img != nullptr);
    Erosion(*img, ksz);
    emit isDone();
}

void ImageProcessor::IncreaseGo(QImage *img, int ksz)
{
    assert(img != nullptr);
    Increase(*img, ksz);
    emit isDone();
}

void ImageProcessor::RotateLeftGo(QImage* img)
{
    assert(img != nullptr);
    RotateLeft(*img);
    emit isDone();
}

void ImageProcessor::RotateRightGo(QImage *img)
{
    assert(img != nullptr);
    RotateRight(*img);
    emit isDone();
}

void ImageProcessor::HMirrorGo(QImage *img)
{
    assert(img != nullptr);
    *img = img->mirrored(false, true);
    emit isDone();
}

void ImageProcessor::VMirrorGo(QImage *img)
{
    assert(img != nullptr);
    *img = img->mirrored(true, false);
    emit isDone();
}

void ImageProcessor::GrayWorldGo(QImage *img)
{
    assert(img != nullptr);
    GrayWorld(*img);
    emit isDone();
}

void ImageProcessor::LinearCorrGo(QImage *img)
{
    assert(img != nullptr);
    LinearCorrection(*img);
    emit isDone();
}

void ImageProcessor::GammaFuncGo(QImage *img, double c, double d)
{
    assert(img != nullptr);
    GammaFunc(*img, c, d);
    emit isDone();
}

void ImageProcessor::GaussBlurGo(QImage *img)
{
    assert(img != nullptr);
    GaussBlur(*img);
    emit isDone();
}
