#include "imageproc.h"

#include <utility>
#include <future>
#include <cmath>

#include "timer.h"

template<typename F, typename... Args>
void ForEachPixel(MyColorIterator first, MyColorIterator last, F func, Args&&... args)
{
    while(first != last)
    {
        *first = func(*first, std::forward<Args>(args)...);
        ++first;
    }
}

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

inline Uint8 ovfctrl(const int x) noexcept
{
    if(x > 255) return 255;
    if(x < 0) return 0;
    return x;
}

inline int b_ctrl(int& x, const int max) noexcept
{
    if(x < 0)		return x = x * (-1) - 1;
    if(x >= max)	return x = max - (x - (max - 1));

    return x;
}

Uint8 find_median(Matrix<Uint8>& m, array<int, 256>& hist, bool ns)
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

    int count = 0;
    Uint8 result = 0;
    int ksz_2_2 = ksz * ksz / 2;
    for (int k = 0; k < 256; k++)
    {
        count += hist[k];
        if (count > ksz_2_2){
            result = k;
            break;
        }
    }

    for (int i = 0; i < ksz; i++)
        --hist[m[i][0]];

    return result;
}

Uint8 find_min(Matrix<Uint8>& m, array<int, 256>& hist, bool ns)
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

    for(int i = 0; i < 256; i++)
    {
        if(hist[i] != 0){
            result = i;
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
        if(hist[i] != 0){
            result = i;
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
            int posPixX = i - ksz_2 + x;
            int posPixY = j - ksz_2 + y;

            b_ctrl(posPixX, width);
            b_ctrl(posPixY, height);

            QRgb tmpc = img->pixel(posPixX, posPixY);

            red[x][y] = qRed(tmpc);
            green[x][y] = qGreen(tmpc);
            blue[x][y] = qBlue(tmpc);
        }
    }
}


ImageProc::ImageProc(QObject *parent):QObject(parent) {}

void ImageProc::rotate_left(QImage *img)
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

void ImageProc::rotate_right(QImage *img)
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

void ImageProc::LinearCorr(QImage* img)
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
        return (curr - min) * 255.0 / div;
    };

    auto first = MyColorIterator::Begin(*img);
    auto last = MyColorIterator::End(*img);

    ForEachPixel(first, last, [&corr, &mmc, &divR, &divG, &divB](QRgb& pixel){
        return qRgb(corr(qRed(pixel), get<0>(mmc).first, divR),
                    corr(qGreen(pixel), get<1>(mmc).first, divG),
                    corr(qBlue(pixel), get<2>(mmc).first, divB));
    });
}

void ImageProc::GrayWorld(QImage* img)
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

    ForEachPixel(first, last, [&avgR, &avgG, &avgB](QRgb& pixel){
        return qRgb(ovfctrl(qRed(pixel) * avgR),
                    ovfctrl(qGreen(pixel) * avgG),
                    ovfctrl(qBlue(pixel) * avgB));
    });
}

void ImageProc::GammaFunc(QImage* img, double c, double d)
{
    if(img->isNull())
        return;

    auto first = MyColorIterator::Begin(*img);
    auto last = MyColorIterator::End(*img);

    ForEachPixel(first, last, [&c, &d](QRgb& pixel){
        return qRgb(ovfctrl(round(c * pow(qRed(pixel), d))),
                    ovfctrl(round(c * pow(qGreen(pixel), d))),
                    ovfctrl(round(c * pow(qBlue(pixel), d)))
                    );
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

            int r = inner_product(part_r.cbegin(), part_r.cend(), kernel.cbegin(), 0.0) / div;
            int g = inner_product(part_g.cbegin(), part_g.cend(), kernel.cbegin(), 0.0) / div;
            int b = inner_product(part_b.cbegin(), part_b.cend(), kernel.cbegin(), 0.0) / div;

            img->setPixel(i, j, qRgb(r, g, b));
        }
    }
}

void ImageProc::GaussBlur(QImage* img)
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

//    Matrix<Uint8> part_r(ksz, ksz);
//    Matrix<Uint8> part_g(ksz, ksz);
//    Matrix<Uint8> part_b(ksz, ksz);

//    for (int i = 0; i < width; i++)
//    {
//        for (int j = 0; j < height; j++)
//        {
//            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

//            int r = inner_product(part_r.cbegin(), part_r.cend(), kernel.cbegin(), 0.0) / div;
//            int g = inner_product(part_g.cbegin(), part_g.cend(), kernel.cbegin(), 0.0) / div;
//            int b = inner_product(part_b.cbegin(), part_b.cend(), kernel.cbegin(), 0.0) / div;

//            img->setPixel(i, j, qRgb(r, g, b));
//        }
//    }

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

            bool is_new_line = (j == 0);

            QRgb tmp = qRgb(find_median(part_r, hist_r, is_new_line),
                            find_median(part_g, hist_g, is_new_line),
                            find_median(part_b, hist_b, is_new_line)
                            );

            new_img.setPixel(i, j, tmp);
        }
    }
}

void ImageProc::MedianFilter(QImage* img, const int ksz)
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

//    Matrix<Uint8> part_r(ksz, ksz);
//    Matrix<Uint8> part_g(ksz, ksz);
//    Matrix<Uint8> part_b(ksz, ksz);

//    const int szg = 256;
//    array<int, szg> hist_r{ 0 };
//    array<int, szg> hist_g{ 0 };
//    array<int, szg> hist_b{ 0 };

//    for (int i = 0; i < width; i++)
//    {
//        for (int j = 0; j < height; j++)
//        {
//            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

//            bool is_new_line = (j == 0);

//            QRgb tmp = qRgb(find_median(part_r, hist_r, is_new_line),
//                            find_median(part_g, hist_g, is_new_line),
//                            find_median(part_b, hist_b, is_new_line)
//                            );

//            new_img.setPixel(i, j, tmp);
//        }
//    }

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

            int r = inner_product(part_r.cbegin(), part_r.cend(), kernel->cbegin(), 0.0) / div;
            int g = inner_product(part_g.cbegin(), part_g.cend(), kernel->cbegin(), 0.0) / div;
            int b = inner_product(part_b.cbegin(), part_b.cend(), kernel->cbegin(), 0.0) / div;

            QRgb tmpc = qRgb(ovfctrl(r), ovfctrl(g), ovfctrl(b));

            new_img.setPixel(i, j, tmpc);
        }
    }
}

void ImageProc::CustomFilter(QImage *img, vector<double>* kernel)
{
    if(img->isNull())
        return;

    int width = img->width();
    int height = img->height();
    const int ksz = static_cast<int>(sqrt(kernel->size()));

    if (ksz % 2 == 0 || ksz < 3 || ksz > width || ksz > height)
        return;

    const double div = accumulate(kernel->cbegin(), kernel->cend(), 0.0);

    QImage new_img(width, height, QImage::Format_RGB32);

    auto f1 = std::async(std::launch::async, CustomFilterLoop, img, std::ref(new_img), kernel, ksz, div, 0, 0, width / 3, height);
    auto f2 = std::async(std::launch::async, CustomFilterLoop, img, std::ref(new_img), kernel, ksz, div, width / 3, 0, (width / 3) * 2, height);
    auto f3 = std::async(std::launch::async, CustomFilterLoop, img, std::ref(new_img), kernel, ksz, div, (width / 3) * 2, 0, width, height);

    f1.wait();
    f2.wait();
    f3.wait();

//    Matrix<Uint8> part_r(ksz, ksz);
//    Matrix<Uint8> part_g(ksz, ksz);
//    Matrix<Uint8> part_b(ksz, ksz);

//    for (int i = 0; i < width; i++)
//    {
//        for (int j = 0; j < height; j++)
//        {
//            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

//            int r = inner_product(part_r.cbegin(), part_r.cend(), kernel->cbegin(), 0.0) / div;
//            int g = inner_product(part_g.cbegin(), part_g.cend(), kernel->cbegin(), 0.0) / div;
//            int b = inner_product(part_b.cbegin(), part_b.cend(), kernel->cbegin(), 0.0) / div;

//            QRgb tmpc = qRgb(ovfctrl(r), ovfctrl(g), ovfctrl(b));

//            new_img.setPixel(i, j, tmpc);
//        }
//    }

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


            new_img.setPixel(i, j, qRgb(
                             find_min(part_r, hist_r, is_new_s),
                             find_min(part_g, hist_g, is_new_s),
                             find_min(part_b, hist_b, is_new_s)
                             ));
        }
    }
}

void ImageProc::Erosion(QImage *img, int ksz)
{
    if(img->isNull())
        return;

    int width = img->width();
    int height = img->height();

    if (ksz % 2 == 0 || ksz < 3 || ksz > width || ksz > height)
        return;

    QImage new_img(width, height, QImage::Format_RGB32);

//    Matrix<Uint8> part_r(ksz, ksz);
//    Matrix<Uint8> part_g(ksz, ksz);
//    Matrix<Uint8> part_b(ksz, ksz);

//    const int szg = 256;
//    array<int, szg> hist_r{ 0 };
//    array<int, szg> hist_g{ 0 };
//    array<int, szg> hist_b{ 0 };

//    for (int i = 0; i < width; i++)
//    {
//        for (int j = 0; j < height; j++)
//        {
//            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

//            bool is_new_s = (j == 0);


//            new_img.setPixel(i, j, qRgb(
//                             find_min(part_r, hist_r, is_new_s),
//                             find_min(part_g, hist_g, is_new_s),
//                             find_min(part_b, hist_b, is_new_s)
//                             ));
//        }
//    }

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

void ImageProc::Increase(QImage *img, int ksz)
{
    if(img->isNull())
        return;

    int width = img->width();
    int height = img->height();

    if (ksz % 2 == 0 || ksz < 3 || ksz > width || ksz > height)
        return;

    QImage new_img(width, height, QImage::Format_RGB32);

//    Matrix<Uint8> part_r(ksz, ksz);
//    Matrix<Uint8> part_g(ksz, ksz);
//    Matrix<Uint8> part_b(ksz, ksz);

//    const int szg = 256;
//    array<int, szg> hist_r{ 0 };
//    array<int, szg> hist_g{ 0 };
//    array<int, szg> hist_b{ 0 };

//    for (int i = 0; i < width; i++)
//    {
//        for (int j = 0; j < height; j++)
//        {
//            fillTmpMatrix(part_r, part_g, part_b, img, ksz, i, j);

//            bool is_new_s = j == 0;


//            new_img.setPixel(i, j, qRgb(
//                             find_max(part_r, hist_r, is_new_s),
//                             find_max(part_g, hist_g, is_new_s),
//                             find_max(part_b, hist_b, is_new_s)
//                             ));
//        }
//    }

    auto f1 = std::async(std::launch::async, IncreaseLoop, img, std::ref(new_img), ksz, 0, 0, width / 3, height);
    auto f2 = std::async(std::launch::async, IncreaseLoop, img, std::ref(new_img), ksz, width / 3, 0, (width / 3) * 2, height);
    auto f3 = std::async(std::launch::async, IncreaseLoop, img, std::ref(new_img), ksz, (width / 3) * 2, 0, width, height);

    f1.wait();
    f2.wait();
    f3.wait();

    *img = move(new_img);
}



void ImageProc::MedianFilterGo(QImage *img, const int ksz)
{
    MedianFilter(img, ksz);
    emit isDone();
}

void ImageProc::CustomFilterGo(QImage *img, vector<double>* kernel)
{
    CustomFilter(img, kernel);
    emit isDone();
}

void ImageProc::ErosionGo(QImage *img, int ksz)
{
    Erosion(img, ksz);
    emit isDone();
}

void ImageProc::IncreaseGo(QImage *img, int ksz)
{
    Increase(img, ksz);
    emit isDone();
}

void ImageProc::RotateLeftGo(QImage* img)
{
    rotate_left(img);
    emit isDone();
}

void ImageProc::RotateRightGo(QImage *img)
{
    rotate_right(img);
    emit isDone();
}

void ImageProc::HMirrorGo(QImage *img)
{
    *img = img->mirrored(false, true);
    emit isDone();
}

void ImageProc::VMirrorGo(QImage *img)
{
    *img = img->mirrored(true, false);
    emit isDone();
}

void ImageProc::GrayWorldGo(QImage *img)
{
    GrayWorld(img);
    emit isDone();
}

void ImageProc::LinearCorrGo(QImage *img)
{
    LinearCorr(img);
    emit isDone();
}

void ImageProc::GammaFuncGo(QImage *img, double c, double d)
{
    GammaFunc(img, c, d);
    emit isDone();
}

void ImageProc::GaussBlurGo(QImage *img)
{
    GaussBlur(img);
    emit isDone();
}
