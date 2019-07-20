#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "mycoloriterator.hpp"
#include "matrix.hpp"

#include <QObject>
#include <QImage>

#include <memory>

class ImageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessor(QObject* parent = nullptr);

signals:
    void isDone();

public slots:
    void GrayWorldGo(QImage* img);
    void LinearCorrGo(QImage* img);
    void GammaFuncGo(QImage* img, double c, double d);
    void GaussBlurGo(QImage* img);
    void MedianFilterGo(QImage* img, const int ksz);
    void CustomFilterGo(QImage *img, const std::vector<double>* kernel);
    void ErosionGo(QImage* img, int ksz);
    void IncreaseGo(QImage* img, int ksz);
    void RotateLeftGo(QImage* img);
    void RotateRightGo(QImage* img);
    void HMirrorGo(QImage* img);
    void VMirrorGo(QImage* img);
};

#endif // IMAGEPROCESSOR_H
