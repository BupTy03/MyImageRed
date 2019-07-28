#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include "mycoloriterator.hpp"
#include "matrix.hpp"

#include <QImage>

#include <tuple>
#include <array>
#include <vector>

constexpr int MAX_COLOR = 256;

using ColorMatrix = Matrix<color_t>;
using Hist = std::vector<std::uint64_t>;
using HistArray = std::array<std::uint64_t, MAX_COLOR>;
using HistRGB = std::tuple<
    std::vector<std::uint64_t>,
    std::vector<std::uint64_t>,
    std::vector<std::uint64_t>
>;

using MinMaxColorPair = std::pair<color_t, color_t>;
using MinMaxColorTuple = std::tuple<
    MinMaxColorPair,
    MinMaxColorPair,
    MinMaxColorPair
>;

void RotateLeft(QImage& img);
void RotateRight(QImage& img);

void GrayWorld(QImage& img);
void LinearCorrection(QImage& img);

void GammaFunc(QImage& img, double c, double d);
void GaussBlur(QImage& img);

void MedianFilter(QImage& img, const int kernelSize);
void CustomFilter(QImage& img, const std::vector<double>& kernel);
void Erosion(QImage& img, const int kernelSize);
void Increase(QImage& img, const int kernelSize);

HistRGB MakeHistogram(QImage& img);

#endif // IMAGEPROCESSING_H
