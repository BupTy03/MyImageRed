#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include "mycoloriterator.h"
#include "matrix.h"

#include <QThread>

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

static void RotateLeft(QImage& img)
{
    if(img.isNull())
        return;

    int width =  img.width();
    int height = img.height();

    QImage new_img(height, width, QImage::Format_RGB32);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            new_img.setPixel(j, width - 1 - i, img.pixel(i, j));
        }
    }

    img = move(new_img);
}

static void RotateRight(QImage& img)
{
    if(img.isNull())
        return;

    int width = img.width();
    int height = img.height();

    QImage new_img(height, width, QImage::Format_RGB32);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            new_img.setPixel(height - 1 - j, i, img.pixel(i, j));
        }
    }

    img = move(new_img);
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

static void GrayWorld(QImage& img)
{
    if(img.isNull())
        return;

    auto Cfirst = ConstMyColorIterator::CBegin(img);
    auto Clast = ConstMyColorIterator::CEnd(img);

    auto summ = ColorSum(Cfirst, Clast);

    const double countPixels = static_cast<double>(img.width() * img.height());

    double avgR = get<0>(summ) / countPixels;
    double avgG = get<1>(summ) / countPixels;
    double avgB = get<2>(summ) / countPixels;

    double avgAll = (avgR + avgG + avgB) / 3.0;

    avgR = avgAll / avgR;
    avgG = avgAll / avgG;
    avgB = avgAll / avgB;

    auto first = MyColorIterator::Begin(img);
    auto last = MyColorIterator::End(img);
    std::transform(first, last, first, [avgR, avgG, avgB](auto pixel){
        return qRgb(OverflowControl(qRed(pixel) * static_cast<int>(avgR)),
                    OverflowControl(qGreen(pixel) * static_cast<int>(avgG)),
                    OverflowControl(qBlue(pixel) * static_cast<int>(avgB)));
    });
}

static void LinearCorrection(QImage& img)
{
    if(img.isNull())
        return;

    auto Cfirst = ConstMyColorIterator::CBegin(img);
    auto Clast = ConstMyColorIterator::CEnd(img);

    auto mmc = MinMaxColor(Cfirst, Clast);

    const double divR = get<0>(mmc).second - get<0>(mmc).first;
    const double divG = get<1>(mmc).second - get<1>(mmc).first;
    const double divB = get<2>(mmc).second - get<2>(mmc).first;

    auto corr = [](const uchar curr, const uchar min, const double div){
        return static_cast<int>((curr - min) * 255.0 / div);
    };

    auto first = MyColorIterator::Begin(img);
    auto last = MyColorIterator::End(img);
    std::transform(first, last, first, [&corr, &mmc, divR, divG, divB](auto pixel){
        return qRgb(corr(static_cast<uchar>(qRed(pixel)), get<0>(mmc).first, divR),
                    corr(static_cast<uchar>(qGreen(pixel)), get<1>(mmc).first, divG),
                    corr(static_cast<uchar>(qBlue(pixel)), get<2>(mmc).first, divB));
    });
}

static void GammaFunc(QImage& img, double c, double d)
{
    if(img.isNull())
        return;

    auto first = MyColorIterator::Begin(img);
    auto last = MyColorIterator::End(img);
    std::transform(first, last, first, [c, d](auto pixel){
        const auto r = OverflowControl(static_cast<int>(round(c * pow(qRed(pixel), d))));
        const auto g = OverflowControl(static_cast<int>(round(c * pow(qGreen(pixel), d))));
        const auto b = OverflowControl(static_cast<int>(round(c * pow(qBlue(pixel), d))));

        return qRgb(r, g, b);
    });
}

/*
 * Dividing the matrix into countThreads equal parts by width
 * to perform calculations on them in different threads.
*/
template<class Function, class... Args>
static void ParallelizeMatrixCalculations(
        const int countThreads,
        const int heightMatrix, const int widthMatrix,
        Function&& func, Args&&... args
        )
{
    assert(countThreads > 0);
    assert(heightMatrix > 0);
    assert(widthMatrix > 0);

    std::vector<std::future<void>> futures;
    const int part = widthMatrix / countThreads;
    for(int i = 1; i <= countThreads; ++i)
    {
        futures.push_back(std::async(std::launch::async, std::forward<Function>(func),
                   0,               // first row
                   (i - 1) * part,  // first column
                   widthMatrix,     // last row
                   i * part,        // last column
                   std::forward<Args>(args)...
        ));
    }

    for(const auto& fut : futures)
    {
        fut.wait();
    }
}

template<const int kernelSize>
static void GaussBlurLoopPart(
        const int beginRow, const int beginCol,
        const int endRow, const int endCol,
        QImage& img, SMatrix<double, kernelSize, kernelSize>& kernel,
        const double divider)
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

    ParallelizeMatrixCalculations(QThread::idealThreadCount(), height, width,
                                  GaussBlurLoopPart<ksz>, std::ref(img), std::ref(kernel), div);

/*
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
*/
}

static void MedianFilterLoopPart(
        const int beginRow, const int beginCol,
        const int endRow, const int endCol,
        const QImage& img, QImage& newImg, const int kernelSize)
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

    ParallelizeMatrixCalculations(QThread::idealThreadCount(), height, width,
                                  MedianFilterLoopPart, std::ref(img), std::ref(newImg), kernelSize);

/*
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
*/
    img = move(newImg);
}

static void CustomFilterLoopPart(
        const int beginRow, const int beginCol,
        const int endRow, const int endCol,
        const QImage& img, QImage& newImg, const std::vector<double>& kernel,
        const int kernelSize, const double divider
)
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

    ParallelizeMatrixCalculations(QThread::idealThreadCount(), height, width,
                                  CustomFilterLoopPart, std::cref(img), std::ref(newImg), std::cref(kernel), kernelSize, divider);

/*
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
*/
    img = move(newImg);
}

static void ErosionLoopPart(
        const int beginRow, const int beginCol,
        const int endRow, const int endCol,
        const QImage& img, QImage& newImg, const int kernelSize
)
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

    ParallelizeMatrixCalculations(QThread::idealThreadCount(), height, width,
                                  ErosionLoopPart, std::cref(img), std::ref(newImg), kernelSize);

/*
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
*/
    img = move(newImg);
}

static void IncreaseLoopPart(
        const int beginRow, const int beginCol,
        const int endRow, const int endCol,
        const QImage& img, QImage& newImg, const int kernelSize
)
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

    ParallelizeMatrixCalculations(QThread::idealThreadCount(), height, width,
                                  IncreaseLoopPart, std::cref(img), std::ref(newImg), kernelSize);
/*
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
*/
    img = move(newImg);
}

} // namespace image_processing

#endif // IMAGEPROCESSING_H
