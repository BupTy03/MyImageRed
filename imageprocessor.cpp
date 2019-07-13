#include "imageprocessor.h"
#include "imageprocessing.h"

#include <utility>
#include <future>
#include <cmath>

#include "timer.h"

#if 0

tuple<pair<uchar, uchar>, pair<uchar, uchar>, pair<uchar, uchar>> MinMaxColor(ConstMyColorIterator first, ConstMyColorIterator last)
{
    uchar red = first.red();
    uchar green = first.green();
    uchar blue = first.blue();

    pair<uchar, uchar> R = make_pair(red, red);
    pair<uchar, uchar> G = make_pair(green, green);
    pair<uchar, uchar> B = make_pair(blue, blue);

    ++first;

    auto comp = [](uchar a, pair<uchar, uchar>& b){
      if(a < b.first)
          b.first = a;
      else if(a > b.second)
          b.second = a;
    };

    while(first != last)
    {
        red = first.red();
        green = first.green();
        blue = first.blue();

        comp(red, R);
        comp(green, G);
        comp(blue, B);

        ++first;
    }

    return make_tuple(R, G, B);
}

tuple<ull, ull, ull> ColorSum(ConstMyColorIterator first, ConstMyColorIterator last)
{
    ull R = 0;
    ull G = 0;
    ull B = 0;

    while(first != last)
    {
        R += first.red();
        G += first.green();
        B += first.blue();

        ++first;
    }

    return make_tuple(R, G, B);
}

inline Uint8 ovfctrl(const int x) noexcept { return static_cast<Uint8>(((x > 255) ? 255 : (x < 0) ? 0 : x)); }

inline int b_ctrl(const int x, const int max) noexcept
{   
    return (x < 0) ? (x * (-1) - 1) : (x >= max) ? (max - (x - (max - 1))) : x;
}

Uint8 find_median(Matrix<Uint8>& m, array<int, 256>& hist, bool ns)
{
    const int ksz = m.size_dim1();

    if (ns)
    {
        hist.fill(0);

        for (const auto i : m)
            ++hist[i];
    }
    else {
        for (int i = 0; i < ksz; i++)
            ++hist[m[i][ksz - 1]];
    }

    int count = 0;
    Uint8 result = 0;
    const int ksz_2_2 = ksz * ksz / 2;
    for (std::size_t k = 0; k < 256; k++)
    {
        count += hist[k];
        if (count > ksz_2_2){
            result = static_cast<Uint8>(k);
            break;
        }
    }

    for (int i = 0; i < ksz; i++)
        --hist[m[i][0]];

    return result;
}

Uint8 find_min(Matrix<Uint8>& m, array<int, 256>& hist, bool ns)
{
    const int ksz = m.size_dim1();

    if (ns)
    {
        hist.fill(0);

        for (const auto i : m)
            ++hist[i];
    }
    else {
        for (int i = 0; i < ksz; i++)
            ++hist[m[i][ksz - 1]];
    }

    Uint8 result = 0;
    for(std::size_t i = 0; i < 256; i++)
    {
        if(hist[i] != 0){
            result = static_cast<Uint8>(i);
            break;
        }
    }

    for (int i = 0; i < ksz; i++)
        --hist[m[i][0]];

    return result;
}

Uint8 find_max(Matrix<Uint8>& m, array<int, 256>& hist, bool ns)
{
    int ksz = m.size_dim1();

    if (ns)
    {
        hist.fill(0);

        for (const auto i : m)
            ++hist[i];
    }
    else {
        for (int i = 0; i < ksz; i++)
            ++hist[m[i][ksz - 1]];
    }

    Uint8 result = 0;
    for(int i = 255; i >= 0; i--)
    {
        if(hist[static_cast<std::size_t>(i)] != 0){
            result = static_cast<Uint8>(i);
            break;
        }
    }

    for (int i = 0; i < ksz; i++)
        --hist[m[i][0]];

    return result;
}

void fillTmpMatrix(Matrix<Uint8>& red, Matrix<Uint8>& green, Matrix<Uint8>& blue, const QImage* img, const int ksz, const int i, const int j)
{
    const int width = img->width();
    const int height = img->height();
    const int ksz_2 = ksz / 2;

    for (int x = 0; x < ksz; x++)
    {
        for (int y = 0; y < ksz; y++)
        {
            const int posPixX = b_ctrl(i - ksz_2 + x, width);
            const int posPixY = b_ctrl(j - ksz_2 + y, height);

            const auto tmpc = img->pixel(posPixX, posPixY);
            red[x][y] = static_cast<Uint8>(qRed(tmpc));
            green[x][y] = static_cast<Uint8>(qGreen(tmpc));
            blue[x][y] = static_cast<Uint8>(qBlue(tmpc));
        }
    }
}


#endif

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{}

#if 0

void ImageProcessor::rotate_left(QImage *img)
{
    if(img->isNull())
        return;

    int width =  img->width();
    int height = img->height();

    QImage new_img(height, width, QImage::Format_RGB32);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            new_img.setPixel(j, width - 1 - i, img->pixel(i, j));
        }
    }

    *img = move(new_img);
}

void ImageProcessor::rotate_right(QImage *img)
{
    if(img->isNull())
        return;

    int width = img->width();
    int height = img->height();

    QImage new_img(height, width, QImage::Format_RGB32);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            new_img.setPixel(height - 1 - j, i, img->pixel(i, j));
        }
    }

    *img = move(new_img);
}

void ImageProcessor::LinearCorr(QImage* img)
{
    if(img->isNull())
        return;

    auto Cfirst = ConstMyColorIterator::CBegin(*img);
    auto Clast = ConstMyColorIterator::CEnd(*img);

    auto mmc = MinMaxColor(Cfirst, Clast);

    const double divR = get<0>(mmc).second - get<0>(mmc).first;
    const double divG = get<1>(mmc).second - get<1>(mmc).first;
    const double divB = get<2>(mmc).second - get<2>(mmc).first;

    auto corr = [](const uchar curr, const uchar min, const double div){
        return static_cast<int>((curr - min) * 255.0 / div);
    };

    auto first = MyColorIterator::Begin(*img);
    auto last = MyColorIterator::End(*img);
    std::transform(first, last, first, [&corr, &mmc, divR, divG, divB](auto pixel){
        return qRgb(corr(static_cast<uchar>(qRed(pixel)), get<0>(mmc).first, divR),
                    corr(static_cast<uchar>(qGreen(pixel)), get<1>(mmc).first, divG),
                    corr(static_cast<uchar>(qBlue(pixel)), get<2>(mmc).first, divB));
    });
}

void ImageProcessor::GrayWorld(QImage* img)
{
    if(img->isNull())
        return;

    auto Cfirst = ConstMyColorIterator::CBegin(*img);
    auto Clast = ConstMyColorIterator::CEnd(*img);

    auto summ = ColorSum(Cfirst, Clast);

    const double countPixels = static_cast<double>(img->width() * img->height());

    double avgR = get<0>(summ) / countPixels;
    double avgG = get<1>(summ) / countPixels;
    double avgB = get<2>(summ) / countPixels;

    double avgAll = (avgR + avgG + avgB) / 3.0;

    avgR = avgAll / avgR;
    avgG = avgAll / avgG;
    avgB = avgAll / avgB;

    auto first = MyColorIterator::Begin(*img);
    auto last = MyColorIterator::End(*img);
    std::transform(first, last, first, [avgR, avgG, avgB](auto pixel){
        return qRgb(ovfctrl(qRed(pixel) * static_cast<int>(avgR)),
                    ovfctrl(qGreen(pixel) * static_cast<int>(avgG)),
                    ovfctrl(qBlue(pixel) * static_cast<int>(avgB)));
    });
}

void ImageProcessor::GammaFunc(QImage* img, double c, double d)
{
    if(img->isNull())
        return;

    auto first = MyColorIterator::Begin(*img);
    auto last = MyColorIterator::End(*img);
    std::transform(first, last, first, [c, d](auto pixel){
        const auto r = ovfctrl(static_cast<int>(round(c * pow(qRed(pixel), d))));
        const auto g = ovfctrl(static_cast<int>(round(c * pow(qGreen(pixel), d))));
        const auto b = ovfctrl(static_cast<int>(round(c * pow(qBlue(pixel), d))));

        return qRgb(r, g, b);
    });
}

template<const Index ksz>
void GaussBlurLoop(QImage* img, SMatrix<double, ksz, ksz>& kernel, const double div,
                   const int begin_x, const int begin_y, const int end_x, const int end_y)
{
    Matrix<Uint8> part_r(ksz, ksz);
    Matrix<Uint8> part_g(ksz, ksz);
    Matrix<Uint8> part_b(ksz, ksz);

    for(int i = begin_x; i < end_x; ++i)
    {
        for(int j = begin_y; j < end_y; ++j)
        {
            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

            const auto r = static_cast<int>(inner_product(part_r.cbegin(), part_r.cend(), kernel.cbegin(), 0.0) / div);
            const auto g = static_cast<int>(inner_product(part_g.cbegin(), part_g.cend(), kernel.cbegin(), 0.0) / div);
            const auto b = static_cast<int>(inner_product(part_b.cbegin(), part_b.cend(), kernel.cbegin(), 0.0) / div);

            img->setPixel(i, j, qRgb(r, g, b));
        }
    }
}

void ImageProcessor::GaussBlur(QImage* img)
{
    if(img->isNull())
        return;

    const int width = img->width();
    const int height = img->height();
    constexpr int ksz = 5;

    SMatrix<double, ksz, ksz> kernel = {
        0.000789, 0.006581, 0.013347, 0.006581, 0.000789,
        0.006581, 0.054901, 0.111345, 0.054901, 0.006581,
        0.013347, 0.111345, 0.225821, 0.111345, 0.013347,
        0.006581, 0.054901, 0.111345, 0.054901, 0.006581,
        0.000789, 0.006581, 0.013347, 0.006581, 0.000789
    };

    double div = std::accumulate(kernel.cbegin(), kernel.cend(), 0.0);
    auto f1 = std::async(std::launch::async, GaussBlurLoop<ksz>, img, std::ref(kernel), div, 0, 0, width, height / 3);
    auto f2 = std::async(std::launch::async, GaussBlurLoop<ksz>, img, std::ref(kernel), div, 0, height / 3, width, (height / 3) * 2);
    auto f3 = std::async(std::launch::async, GaussBlurLoop<ksz>, img, std::ref(kernel), div, 0, (height / 3) * 2, width, height);

    f1.wait();
    f2.wait();
    f3.wait();
}

void MedianFilterLoop(QImage* img, QImage& new_img, const int ksz, const int begin_x, const int begin_y, const int end_x, const int end_y)
{
    Matrix<Uint8> part_r(ksz, ksz);
    Matrix<Uint8> part_g(ksz, ksz);
    Matrix<Uint8> part_b(ksz, ksz);

    constexpr int szg = 256;
    array<int, szg> hist_r{ 0 };
    array<int, szg> hist_g{ 0 };
    array<int, szg> hist_b{ 0 };

    for (int i = begin_x; i < end_x; ++i)
    {
        for (int j = begin_y; j < end_y; ++j)
        {
            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

            const bool is_new_line = (j == 0);

            const auto r = find_median(part_r, hist_r, is_new_line);
            const auto g = find_median(part_g, hist_g, is_new_line);
            const auto b = find_median(part_b, hist_b, is_new_line);

            new_img.setPixel(i, j, qRgb(r, g, b));
        }
    }
}

void ImageProcessor::MedianFilter(QImage* img, const int ksz)
{
    if(img->isNull())
        return;

    int width = img->width();
    int height = img->height();

    if (ksz % 2 == 0 || ksz < 3 || ksz > width || ksz > height)
        return;

    QImage new_img(width, height, QImage::Format_RGB32);

    auto f1 = std::async(std::launch::async, MedianFilterLoop, img, std::ref(new_img),
                         ksz, 0, 0, width / 3, height);

    auto f2 = std::async(std::launch::async, MedianFilterLoop, img, std::ref(new_img),
                         ksz, width / 3, 0, (width / 3) * 2, height);

    auto f3 = std::async(std::launch::async, MedianFilterLoop, img, std::ref(new_img),
                         ksz, (width / 3) * 2, 0, width, height);

    f1.wait();
    f2.wait();
    f3.wait();

    *img = move(new_img);
}

void CustomFilterLoop(QImage* img, QImage& new_img, vector<double>* kernel, const int ksz, const double div,
                      const int begin_x, const int begin_y, const int end_x, const int end_y)
{
    Matrix<Uint8> part_r(ksz, ksz);
    Matrix<Uint8> part_g(ksz, ksz);
    Matrix<Uint8> part_b(ksz, ksz);

    for (int i = begin_x; i < end_x; i++)
    {
        for (int j = begin_y; j < end_y; j++)
        {
            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

            const auto r = static_cast<int>(inner_product(part_r.cbegin(), part_r.cend(), kernel->cbegin(), 0.0) / div);
            const auto g = static_cast<int>(inner_product(part_g.cbegin(), part_g.cend(), kernel->cbegin(), 0.0) / div);
            const auto b = static_cast<int>(inner_product(part_b.cbegin(), part_b.cend(), kernel->cbegin(), 0.0) / div);

            new_img.setPixel(i, j, qRgb(ovfctrl(r), ovfctrl(g), ovfctrl(b)));
        }
    }
}

void ImageProcessor::CustomFilter(QImage *img, vector<double>* kernel)
{
    if(img->isNull())
        return;

    const int width = img->width();
    const int height = img->height();
    const int ksz = static_cast<int>(sqrt(kernel->size()));

    if (ksz % 2 == 0 || ksz < 3 || ksz > width || ksz > height)
        return;

    QImage new_img(width, height, QImage::Format_RGB32);

    const double div = accumulate(kernel->cbegin(), kernel->cend(), 0.0);
    auto f1 = std::async(std::launch::async, CustomFilterLoop, img, std::ref(new_img), kernel, ksz, div, 0, 0, width / 3, height);
    auto f2 = std::async(std::launch::async, CustomFilterLoop, img, std::ref(new_img), kernel, ksz, div, width / 3, 0, (width / 3) * 2, height);
    auto f3 = std::async(std::launch::async, CustomFilterLoop, img, std::ref(new_img), kernel, ksz, div, (width / 3) * 2, 0, width, height);

    f1.wait();
    f2.wait();
    f3.wait();

    *img = move(new_img);
}

void ErosionLoop(QImage* img, QImage& new_img, const int ksz,
                 const int begin_x, const int begin_y, const int end_x, const int end_y)
{
    Matrix<Uint8> part_r(ksz, ksz);
    Matrix<Uint8> part_g(ksz, ksz);
    Matrix<Uint8> part_b(ksz, ksz);

    constexpr int szg = 256;
    array<int, szg> hist_r{ 0 };
    array<int, szg> hist_g{ 0 };
    array<int, szg> hist_b{ 0 };

    for (int i = begin_x; i < end_x; ++i)
    {
        for (int j = begin_y; j < end_y; ++j)
        {
            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

            bool is_new_s = (j == 0);

            const auto r = find_min(part_r, hist_r, is_new_s);
            const auto g = find_min(part_g, hist_g, is_new_s);
            const auto b = find_min(part_b, hist_b, is_new_s);

            new_img.setPixel(i, j, qRgb(r, g, b));
        }
    }
}

void ImageProcessor::Erosion(QImage *img, int ksz)
{
    if(img->isNull())
        return;

    int width = img->width();
    int height = img->height();

    if (ksz % 2 == 0 || ksz < 3 || ksz > width || ksz > height)
        return;

    QImage new_img(width, height, QImage::Format_RGB32);

    auto f1 = std::async(std::launch::async, ErosionLoop, img, std::ref(new_img), ksz, 0, 0, width / 3, height);
    auto f2 = std::async(std::launch::async, ErosionLoop, img, std::ref(new_img), ksz, width / 3, 0, (width / 3) * 2, height);
    auto f3 = std::async(std::launch::async, ErosionLoop, img, std::ref(new_img), ksz, (width / 3) * 2, 0, width, height);

    f1.wait();
    f2.wait();
    f3.wait();

    *img = move(new_img);
}

void IncreaseLoop(QImage* img, QImage& new_img, const int ksz,
                  const int begin_x, const int begin_y, const int end_x, const int end_y)
{
    Matrix<Uint8> part_r(ksz, ksz);
    Matrix<Uint8> part_g(ksz, ksz);
    Matrix<Uint8> part_b(ksz, ksz);

    constexpr int szg = 256;
    array<int, szg> hist_r{ 0 };
    array<int, szg> hist_g{ 0 };
    array<int, szg> hist_b{ 0 };

    for (int i = begin_x; i < end_x; ++i)
    {
        for (int j = begin_y; j < end_y; ++j)
        {
            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

            bool is_new_s = j == 0;


            new_img.setPixel(i, j, qRgb(
                             find_max(part_r, hist_r, is_new_s),
                             find_max(part_g, hist_g, is_new_s),
                             find_max(part_b, hist_b, is_new_s)
                             ));
        }
    }
}

void ImageProcessor::Increase(QImage *img, int ksz)
{
    if(img->isNull())
        return;

    int width = img->width();
    int height = img->height();

    if (ksz % 2 == 0 || ksz < 3 || ksz > width || ksz > height)
        return;

    QImage new_img(width, height, QImage::Format_RGB32);

    auto f1 = std::async(std::launch::async, IncreaseLoop, img, std::ref(new_img), ksz, 0, 0, width / 3, height);
    auto f2 = std::async(std::launch::async, IncreaseLoop, img, std::ref(new_img), ksz, width / 3, 0, (width / 3) * 2, height);
    auto f3 = std::async(std::launch::async, IncreaseLoop, img, std::ref(new_img), ksz, (width / 3) * 2, 0, width, height);

    f1.wait();
    f2.wait();
    f3.wait();

    *img = move(new_img);
}

#endif

void ImageProcessor::MedianFilterGo(QImage *img, const int ksz)
{
    image_processing::MedianFilter(*img, ksz);
    //MedianFilter(img, ksz);
    emit isDone();
}

void ImageProcessor::CustomFilterGo(QImage *img, vector<double>* kernel)
{
    image_processing::CustomFilter(*img, *kernel);
    //CustomFilter(img, kernel);
    emit isDone();
}

void ImageProcessor::ErosionGo(QImage *img, int ksz)
{
    image_processing::Erosion(*img, ksz);
    //Erosion(img, ksz);
    emit isDone();
}

void ImageProcessor::IncreaseGo(QImage *img, int ksz)
{
    image_processing::Increase(*img, ksz);
    //Increase(img, ksz);
    emit isDone();
}

void ImageProcessor::RotateLeftGo(QImage* img)
{
    image_processing::RotateLeft(*img);
    //rotate_left(img);
    emit isDone();
}

void ImageProcessor::RotateRightGo(QImage *img)
{
    image_processing::RotateRight(*img);
    //rotate_right(img);
    emit isDone();
}

void ImageProcessor::HMirrorGo(QImage *img)
{
    *img = img->mirrored(false, true);
    emit isDone();
}

void ImageProcessor::VMirrorGo(QImage *img)
{
    *img = img->mirrored(true, false);
    emit isDone();
}

void ImageProcessor::GrayWorldGo(QImage *img)
{
    image_processing::GrayWorld(*img);
    //GrayWorld(img);
    emit isDone();
}

void ImageProcessor::LinearCorrGo(QImage *img)
{
    image_processing::LinearCorrection(*img);
    //LinearCorr(img);
    emit isDone();
}

void ImageProcessor::GammaFuncGo(QImage *img, double c, double d)
{
    image_processing::GammaFunc(*img, c, d);
    //GammaFunc(img, c, d);
    emit isDone();
}

void ImageProcessor::GaussBlurGo(QImage *img)
{
    image_processing::GaussBlur(*img);
    //GaussBlur(img);
    emit isDone();
}
