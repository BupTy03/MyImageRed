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
    void isDone(std::shared_ptr<QImage>);

public slots:
    void GrayWorldGo(const QImage& img);
    void LinearCorrGo(const QImage& img);
    void GammaFuncGo(const QImage& img, double c, double d);
    void GaussBlurGo(const QImage& img);
    void MedianFilterGo(const QImage& img, const int ksz);
    void CustomFilterGo(const QImage& img, const std::vector<double>& kernel);
    void ErosionGo(const QImage& img, int ksz);
    void IncreaseGo(const QImage& img, int ksz);
    void RotateLeftGo(const QImage& img);
    void RotateRightGo(const QImage& img);
    void HMirrorGo(const QImage& img);
    void VMirrorGo(const QImage& img);
};

#endif // IMAGEPROCESSOR_H
