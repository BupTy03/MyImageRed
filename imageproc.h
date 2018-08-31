#ifndef IMAGEPROC_H
#define IMAGEPROC_H

#include <QObject>
#include <QImage>
#include <QRgb>

#include <utility>
#include <memory>
#include <tuple>

#include "mycoloriterator.h"
#include "matrix.h"

using ull = unsigned long long;
using Uint8 = unsigned char;

using namespace std;

class ImageProc : public QObject
{
    Q_OBJECT
public:
    explicit ImageProc(QObject* parent = nullptr);

private:
    void rotate_left(QImage* img);
    void rotate_right(QImage* img);

    void GrayWorld(QImage* img);
    void LinearCorr(QImage* img);
    void GammaFunc(QImage* img, double c, double d);
    void GaussBlur(QImage* img);
    void MedianFilter(QImage* img, const int ksz);
    void CustomFilter(QImage* img, vector<double> *kernel);
    void Erosion(QImage* img, int ksz);
    void Increase(QImage* img, int ksz);

signals:
    void isDone();

public slots:
    void GrayWorldGo(QImage* img);
    void LinearCorrGo(QImage* img);
    void GammaFuncGo(QImage* img, double c, double d);
    void GaussBlurGo(QImage* img);
    void MedianFilterGo(QImage* img, const int ksz);
    void CustomFilterGo(QImage* img, vector<double>* kernel);
    void ErosionGo(QImage* img, int ksz);
    void IncreaseGo(QImage* img, int ksz);
    void RotateLeftGo(QImage* img);
    void RotateRightGo(QImage* img);
    void HMirrorGo(QImage* img);
    void VMirrorGo(QImage* img);
};

#endif // IMAGEPROC_H
