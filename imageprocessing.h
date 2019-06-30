#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include "mycoloriterator.h"
#include "matrix.h"

#include <algorithm>
#include <utility>
#include <tuple>
#include <array>
#include <vector>
#include <future>
#include <cmath>

namespace image_processing {

enum { MAX_COLOR = 256 };

using std::int64_t;
using std::uint64_t;

using color_t = std::uint8_t;
using ColorMatrix = Matrix<color_t>;
using HistArray = std::array<std::uint64_t, MAX_COLOR>;

using MinMaxColorPair = std::pair<color_t, color_t>;
using MinMaxColorTuple = std::tuple<
    MinMaxColorPair,
    MinMaxColorPair,
    MinMaxColorPair
>;

template<class F, class... Args>
static void ForEachPixel(MyColorIterator first, MyColorIterator last, F func, Args&&... args)
{
    while(first != last)
    {
        *first = func(*first, std::forward<Args>(args)...);
        ++first;
    }
}

static void SetMinOrMaxColor(color_t currentColor, MinMaxColorPair& minMaxPair)
{
    if(currentColor < minMaxPair.first)
        minMaxPair.first = currentColor;
    else if(currentColor > minMaxPair.second)
        minMaxPair.second = currentColor;
}

static MinMaxColorTuple MinMaxColor(ConstMyColorIterator first, ConstMyColorIterator last)
{
    using std::make_pair;

    color_t red = first.red();
    color_t green = first.green();
    color_t blue = first.blue();
    ++first;

    MinMaxColorPair R = make_pair(red, red);
    MinMaxColorPair G = make_pair(green, green);
    MinMaxColorPair B = make_pair(blue, blue);

    while(first != last)
    {
        red = first.red();
        green = first.green();
        blue = first.blue();

        SetMinOrMaxColor(red, R);
        SetMinOrMaxColor(green, G);
        SetMinOrMaxColor(blue, B);

        ++first;
    }

    return std::make_tuple(R, G, B);
}

static std::tuple<uint64_t, uint64_t, uint64_t> ColorSum(ConstMyColorIterator first, ConstMyColorIterator last)
{
    uint64_t sumRed = 0;
    uint64_t sumGreen = 0;
    uint64_t sumBlue = 0;

    for(; first != last; ++first)
    {
        sumRed += first.red();
        sumGreen += first.green();
        sumBlue += first.blue();
    }

    return std::make_tuple(sumRed, sumGreen, sumBlue);
}

static inline color_t OverflowControl(const int x) noexcept
{
    return static_cast<color_t>(((x > 255) ? 255 : (x < 0) ? 0 : x));
}

static inline int BoundsControl(const int x, const int max) noexcept
{
    return (x < 0) ? (x * (-1) - 1) : (x >= max) ? (max - (x - (max - 1))) : x;
}

static color_t FindMedian(const ColorMatrix& m, HistArray& hist, const bool newLine)
{
    const int ksz = m.size_dim1();

    if (newLine)
    {
        hist.fill(0);

        for (auto i : m)
            ++hist[i];
    }
    else {
        for (int i = 0; i < ksz; i++)
            ++hist[m[i][ksz - 1]];
    }

    int count = 0;
    color_t result = 0;
    const int ksz_2_2 = ksz * ksz / 2;
    for (std::size_t k = 0; k < MAX_COLOR; k++)
    {
        count += hist[k];
        if (count > ksz_2_2){
            result = static_cast<color_t>(k);
            break;
        }
    }

    for (int i = 0; i < ksz; i++)
        --hist[m[i][0]];

    return result;
}

static color_t FindMin(const ColorMatrix& m, HistArray& hist, const bool newLine)
{
    const int ksz = m.size_dim1();

    if (newLine)
    {
        hist.fill(0);

        for (auto i : m)
            ++hist[i];
    }
    else {
        for (int i = 0; i < ksz; i++)
            ++hist[m[i][ksz - 1]];
    }

    color_t result = 0;
    for(std::size_t i = 0; i < MAX_COLOR; i++)
    {
        if(hist[i] != 0){
            result = static_cast<color_t>(i);
            break;
        }
    }

    for (int i = 0; i < ksz; i++)
        --hist[m[i][0]];

    return result;
}

static color_t FindMax(const ColorMatrix& m, HistArray& hist, const bool newLine)
{
    int ksz = m.size_dim1();

    if (newLine)
    {
        hist.fill(0);

        for (const auto i : m)
            ++hist[i];
    }
    else {
        for (int i = 0; i < ksz; i++)
            ++hist[m[i][ksz - 1]];
    }

    color_t result = 0;
    for(int i = MAX_COLOR - 1; i >= 0; i--)
    {
        if(hist[static_cast<std::size_t>(i)] != 0){
            result = static_cast<color_t>(i);
            break;
        }
    }

    for (int i = 0; i < ksz; i++)
        --hist[m[i][0]];

    return result;
}


static void FillTmpMatrix(const QImage& img,
                   ColorMatrix& red, ColorMatrix& green, ColorMatrix& blue,
                   const int kernelSize,
                   const int currentRow, const int currentCol)
{
    const int width = img.width();
    const int height = img.height();
    const int halfKernelSize = kernelSize / 2;

    for (int row = 0; row < kernelSize; row++)
    {
        for (int col = 0; col < kernelSize; col++)
        {
            const int posPixX = BoundsControl(currentRow - halfKernelSize + row, width);
            const int posPixY = BoundsControl(currentCol - halfKernelSize + col, height);

            const auto tmpc = img.pixel(posPixX, posPixY);
            red[row][col] = static_cast<color_t>(qRed(tmpc));
            green[row][col] = static_cast<color_t>(qGreen(tmpc));
            blue[row][col] = static_cast<color_t>(qBlue(tmpc));
        }
    }
}


template<const int kernelSize>
static void GaussBlurLoopPart(QImage& img, SMatrix<double, kernelSize, kernelSize>& kernel,
                       const double divider,
                       const int beginRow, const int beginCol,
                       const int endRow, const int endCol)
{
    ColorMatrix partR(kernelSize, kernelSize);
    ColorMatrix partG(kernelSize, kernelSize);
    ColorMatrix partB(kernelSize, kernelSize);

    for(int row = beginRow; row < endRow; ++row)
    {
        for(int col = beginCol; col < endCol; ++col)
        {
            FillTmpMatrix(img, partR, partG, partB, kernelSize, row, col);

            const auto r = static_cast<int>(std::inner_product(partR.cbegin(), partR.cend(), kernel.cbegin(), 0.0) / divider);
            const auto g = static_cast<int>(std::inner_product(partG.cbegin(), partG.cend(), kernel.cbegin(), 0.0) / divider);
            const auto b = static_cast<int>(std::inner_product(partB.cbegin(), partB.cend(), kernel.cbegin(), 0.0) / divider);

            img.setPixel(row, col, qRgb(OverflowControl(r), OverflowControl(g), OverflowControl(b)));
        }
    }
}

static void GaussBlur(QImage& img)
{
    if(img.isNull())
        return;

    const int width = img.width();
    const int height = img.height();
    constexpr int ksz = 5;

    SMatrix<double, ksz, ksz> kernel = {
        0.000789, 0.006581, 0.013347, 0.006581, 0.000789,
        0.006581, 0.054901, 0.111345, 0.054901, 0.006581,
        0.013347, 0.111345, 0.225821, 0.111345, 0.013347,
        0.006581, 0.054901, 0.111345, 0.054901, 0.006581,
        0.000789, 0.006581, 0.013347, 0.006581, 0.000789
    };

    double div = std::accumulate(kernel.cbegin(), kernel.cend(), 0.0);

    const int heightPart = height / 4;
    const int twoHeightParts = heightPart * 2;
    const int threeHeightParts = heightPart * 3;

    auto f1 = std::async(std::launch::async, GaussBlurLoopPart<ksz>, std::ref(img), std::ref(kernel), div, 0, 0, width, heightPart);
    auto f2 = std::async(std::launch::async, GaussBlurLoopPart<ksz>, std::ref(img), std::ref(kernel), div, 0, heightPart, width, twoHeightParts);
    auto f3 = std::async(std::launch::async, GaussBlurLoopPart<ksz>, std::ref(img), std::ref(kernel), div, 0, twoHeightParts, width, threeHeightParts);
    auto f4 = std::async(std::launch::async, GaussBlurLoopPart<ksz>, std::ref(img), std::ref(kernel), div, 0, threeHeightParts, width, height);

    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();
}

static void MedianFilterLoopPart(const QImage& img, QImage& newImg,
                          const int kernelSize,
                          const int beginRow, const int beginCol,
                          const int endRow, const int endCol)
{
    ColorMatrix partR(kernelSize, kernelSize);
    ColorMatrix partG(kernelSize, kernelSize);
    ColorMatrix partB(kernelSize, kernelSize);

    HistArray histR{ 0 };
    HistArray histG{ 0 };
    HistArray histB{ 0 };

    for (int row = beginRow; row < endRow; ++row)
    {
        for (int col = beginCol; col < endCol; ++col)
        {
            FillTmpMatrix(img, partR, partG, partB, kernelSize, row, col);

            const bool isNewLine = (col == 0);

            const auto r = FindMedian(partR, histR, isNewLine);
            const auto g = FindMedian(partG, histG, isNewLine);
            const auto b = FindMedian(partB, histB, isNewLine);

            newImg.setPixel(row, col, qRgb(r, g, b));
        }
    }
}

static void MedianFilter(QImage& img, const int kernelSize)
{
    if(img.isNull())
        return;

    const int width = img.width();
    const int height = img.height();

    if (kernelSize % 2 == 0 || kernelSize < 3 || kernelSize > width || kernelSize > height)
        return;

    QImage newImg(width, height, QImage::Format_RGB32);

    const int heightPart = height / 4;
    const int twoHeightParts = heightPart * 2;
    const int threeHeightParts = heightPart * 3;

    auto f1 = std::async(std::launch::async, MedianFilterLoopPart, std::ref(img), std::ref(newImg), kernelSize, 0, 0, width, heightPart);
    auto f2 = std::async(std::launch::async, MedianFilterLoopPart, std::ref(img), std::ref(newImg), kernelSize, 0, heightPart, width, twoHeightParts);
    auto f3 = std::async(std::launch::async, MedianFilterLoopPart, std::ref(img), std::ref(newImg), kernelSize, 0, twoHeightParts, width, threeHeightParts);
    auto f4 = std::async(std::launch::async, MedianFilterLoopPart, std::ref(img), std::ref(newImg), kernelSize, 0, threeHeightParts, width, height);

    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();

    img = move(newImg);
}

static void CustomFilterLoopPart(const QImage& img, QImage& newImg, const std::vector<double>& kernel,
                          const int kernelSize, const double divider,
                          const int beginRow, const int beginCol,
                          const int endRow, const int endCol)
{
    ColorMatrix partR(kernelSize, kernelSize);
    ColorMatrix partG(kernelSize, kernelSize);
    ColorMatrix partB(kernelSize, kernelSize);

    for (int row = beginRow; row < endRow; row++)
    {
        for (int col = beginCol; col < endCol; col++)
        {
            FillTmpMatrix(img, partR, partG, partB, kernelSize, row, col);

            const auto r = static_cast<int>(std::inner_product(partR.cbegin(), partR.cend(), kernel.cbegin(), 0.0) / divider);
            const auto g = static_cast<int>(std::inner_product(partG.cbegin(), partG.cend(), kernel.cbegin(), 0.0) / divider);
            const auto b = static_cast<int>(std::inner_product(partB.cbegin(), partB.cend(), kernel.cbegin(), 0.0) / divider);

            newImg.setPixel(row, col, qRgb(OverflowControl(r), OverflowControl(g), OverflowControl(b)));
        }
    }
}

static void CustomFilter(QImage& img, const std::vector<double>& kernel)
{
    if(img.isNull())
        return;

    const int width = img.width();
    const int height = img.height();
    const int kernelSize = static_cast<int>(sqrt(kernel.size()));

    if (kernelSize % 2 == 0 || kernelSize < 3 || kernelSize > width || kernelSize > height)
        return;

    QImage newImg(width, height, QImage::Format_RGB32);

    const double divider = std::accumulate(kernel.cbegin(), kernel.cend(), 0.0);

    const int heightPart = height / 4;
    const int twoHeightParts = heightPart * 2;
    const int threeHeightParts = heightPart * 3;

    auto f1 = std::async(std::launch::async, CustomFilterLoopPart, std::cref(img), std::ref(newImg), std::cref(kernel), kernelSize, divider, 0, 0, width, heightPart);
    auto f2 = std::async(std::launch::async, CustomFilterLoopPart, std::cref(img), std::ref(newImg), std::cref(kernel), kernelSize, divider, 0, heightPart, width, twoHeightParts);
    auto f3 = std::async(std::launch::async, CustomFilterLoopPart, std::cref(img), std::ref(newImg), std::cref(kernel), kernelSize, divider, 0, twoHeightParts, width, threeHeightParts);
    auto f4 = std::async(std::launch::async, CustomFilterLoopPart, std::cref(img), std::ref(newImg), std::cref(kernel), kernelSize, divider, 0, threeHeightParts, width, height);

    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();

    img = move(newImg);
}

static void ErosionLoopPart(const QImage& img, QImage& newImg,
                     const int kernelSize,
                     const int beginRow, const int beginCol,
                     const int endRow, const int endCol)
{
    ColorMatrix partR(kernelSize, kernelSize);
    ColorMatrix partG(kernelSize, kernelSize);
    ColorMatrix partB(kernelSize, kernelSize);

    HistArray histR{ 0 };
    HistArray histG{ 0 };
    HistArray histB{ 0 };

    for (int row = beginRow; row < endRow; ++row)
    {
        for (int col = beginCol; col < endCol; ++col)
        {
            FillTmpMatrix(img, partR, partG, partB, kernelSize, row, col);

            bool isNewLine = (col == 0);

            const auto r = FindMin(partR, histR, isNewLine);
            const auto g = FindMin(partG, histG, isNewLine);
            const auto b = FindMin(partB, histB, isNewLine);

            newImg.setPixel(row, col, qRgb(r, g, b));
        }
    }
}

static void Erosion(QImage& img, const int kernelSize)
{
    if(img.isNull())
        return;

    int width = img.width();
    int height = img.height();

    if (kernelSize % 2 == 0 || kernelSize < 3 || kernelSize > width || kernelSize > height)
        return;

    QImage newImg(width, height, QImage::Format_RGB32);

    const int heightPart = height / 4;
    const int twoHeightParts = heightPart * 2;
    const int threeHeightParts = heightPart * 3;

    auto f1 = std::async(std::launch::async, ErosionLoopPart, std::cref(img), std::ref(newImg), kernelSize, 0, 0, width, heightPart);
    auto f2 = std::async(std::launch::async, ErosionLoopPart, std::cref(img), std::ref(newImg), kernelSize, 0, heightPart, width, twoHeightParts);
    auto f3 = std::async(std::launch::async, ErosionLoopPart, std::cref(img), std::ref(newImg), kernelSize, 0, twoHeightParts, width, threeHeightParts);
    auto f4 = std::async(std::launch::async, ErosionLoopPart, std::cref(img), std::ref(newImg), kernelSize, 0, threeHeightParts, width, height);

    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();

    img = move(newImg);
}

static void IncreaseLoopPart(const QImage& img, QImage& newImg,
                      const int kernelSize,
                      const int beginRow, const int beginCol,
                      const int endRow, const int endCol)
{
    ColorMatrix partR(kernelSize, kernelSize);
    ColorMatrix partG(kernelSize, kernelSize);
    ColorMatrix partB(kernelSize, kernelSize);

    HistArray histR{ 0 };
    HistArray histG{ 0 };
    HistArray histB{ 0 };

    for (int row = beginRow; row < endRow; ++row)
    {
        for (int col = beginCol; col < endCol; ++col)
        {
            FillTmpMatrix(img, partR, partG, partB, kernelSize, row, col);

            const bool isNewLine = (col == 0);

            const auto r = FindMax(partR, histR, isNewLine);
            const auto g = FindMax(partG, histG, isNewLine);
            const auto b = FindMax(partB, histB, isNewLine);

            newImg.setPixel(row, col, qRgb(r, g, b));
        }
    }
}

static void Increase(QImage& img, const int kernelSize)
{
    if(img.isNull())
        return;

    int width = img.width();
    int height = img.height();

    if (kernelSize % 2 == 0 || kernelSize < 3 || kernelSize > width || kernelSize > height)
        return;

    QImage newImg(width, height, QImage::Format_RGB32);

    const int heightPart = height / 4;
    const int twoHeightParts = heightPart * 2;
    const int threeHeightParts = heightPart * 3;

    auto f1 = std::async(std::launch::async, IncreaseLoopPart, std::cref(img), std::ref(newImg), kernelSize, 0, 0, width, heightPart);
    auto f2 = std::async(std::launch::async, IncreaseLoopPart, std::cref(img), std::ref(newImg), kernelSize, 0, heightPart, width, twoHeightParts);
    auto f3 = std::async(std::launch::async, IncreaseLoopPart, std::cref(img), std::ref(newImg), kernelSize, 0, twoHeightParts, width, threeHeightParts);
    auto f4 = std::async(std::launch::async, IncreaseLoopPart, std::cref(img), std::ref(newImg), kernelSize, 0, threeHeightParts, width, height);

    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();

    img = move(newImg);
}

} // namespace image_processing

#endif // IMAGEPROCESSING_H
