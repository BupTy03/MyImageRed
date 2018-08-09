#ifndef MYIMAGEPROC_H
#define MYIMAGEPROC_H
/*
#include <QRgb>
#include <QImage>
#include <QDebug>
#include <tuple>
#include <utility>
#include <array>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "mycoloriterator.h"
#include "matrix.h"
#include "mainwindow.h"

using ull = unsigned long long;
using Uint8 = unsigned char;


std::tuple<std::pair<uchar, uchar>, std::pair<uchar, uchar>, std::pair<uchar, uchar>> MinMaxColor(MyColorIterator first, MyColorIterator last)
{
    uchar red = first.red();
    uchar green = first.green();
    uchar blue = first.blue();

    std::pair<uchar, uchar> R = std::make_pair(red, red);
    std::pair<uchar, uchar> G = std::make_pair(green, green);
    std::pair<uchar, uchar> B = std::make_pair(blue, blue);

    ++first;

    auto comp = [](uchar a, std::pair<uchar, uchar>& b){
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

        first++;
    }

    return std::make_tuple(R, G, B);
}

std::tuple<ull, ull, ull> colorSum(MyColorIterator first, MyColorIterator last)
{
    ull R = 0;
    ull G = 0;
    ull B = 0;

    while(first != last)
    {
        R += first.red();
        G += first.green();
        B += first.blue();

        first++;
    }

    return std::make_tuple(R, G, B);
}

void rotate_left(QImage& img)
{
    if(img.isNull())
        return;

    const long width =  img.width();
    const long height = img.height();

    QImage new_img(height, width, QImage::Format_RGB32);

    for (long i = 0; i < width; i++) {
        for (long j = 0; j < height; j++) {
            new_img.setPixelColor(j, width - 1 - i, img.pixelColor(i, j));
        }
    }
}

void rotate_right(QImage& img)
{
    if(img.isNull())
        return;

    const long width = img.width();
    const long height = img.height();

    QImage new_img(height, width, QImage::Format_RGB32);

    for (long i = 0; i < width; i++) {
        for (long j = 0; j < height; j++) {
            new_img.setPixelColor(height - 1 - j, i, img.pixelColor(i, j));
        }
    }
}

void rotate_full(QImage& img)
{
    if(img.isNull())
        return;

    const long width = img.width();
    const long height = img.height();
    long height_2 = height / 2;

    for (long i = 0; i < width; i++) {
        for (long j = 0; j < height_2; j++)
        {
            QColor tmp = img.pixelColor(i, j);
            img.setPixelColor(i, j, img.pixelColor(width - 1 - i, height - 1 - j));
            img.setPixelColor(width - 1 - i, height - 1 - j, tmp);
        }
    }
}

inline int ovfctrl(const int x)
{
    if(x > 255) return 255;
    if(x < 0) return 0;
    return x;
}

bool LinearCorr(QImage& img)
{
    if(img.isNull())
        return false;

    auto first = MyColorIterator::Begin(img);
    auto last = MyColorIterator::End(img);

    auto mmc = MinMaxColor(first, last);

    double divR = std::get<0>(mmc).second - std::get<0>(mmc).first;
    double divG = std::get<1>(mmc).second - std::get<1>(mmc).first;
    double divB = std::get<2>(mmc).second - std::get<2>(mmc).first;

    auto corr = [](const uchar curr, const uchar min, double div){
        return (curr - min) * 255.0 / div;
    };

    while(first != last)
    {
        *first = qRgb(corr(first.red(), std::get<0>(mmc).first, divR),
                      corr(first.green(), std::get<1>(mmc).first, divG),
                      corr(first.blue(), std::get<2>(mmc).first, divB));
        first++;
    }

    return true;
}

bool GrayWorld(QImage& img)
{
    if(img.isNull())
        return false;

    auto first = MyColorIterator::Begin(img);
    auto last = MyColorIterator::End(img);

    double size = img.width() * img.height();

    auto summ = colorSum(first, last);

    double avgR = std::get<0>(summ) / size;
    double avgG = std::get<1>(summ) / size;
    double avgB = std::get<2>(summ) / size;

    double avgAll = (avgR + avgG + avgB) / 3.0;

    avgR = avgAll / avgR;
    avgG = avgAll / avgG;
    avgB = avgAll / avgB;

    while(first != last)
    {
        *first = qRgb(ovfctrl(first.red() * avgR),
                      ovfctrl(first.green() * avgG),
                      ovfctrl(first.blue() * avgB));

        first++;
    }

    return true;
}

int b_ctrl(int& x, const int max)
{
    if(x < 0)		return x = x * (-1) - 1;
    if(x >= max)	return x = max - (x - (max - 1));

    return x;
}

bool GammaCorr(QImage& img, double c = 1.0, double d = 1.0)
{
    if(img.isNull())
        return false;

    auto first = MyColorIterator::Begin(img);
    auto last = MyColorIterator::End(img);

    while(first != last)
    {
        *first = qRgb(ovfctrl(round(c * pow(first.red(), d))),
                      ovfctrl(round(c * pow(first.green(), d))),
                      ovfctrl(round(c * pow(first.blue(), d)))
                     );

        first++;
    }

    return true;
}

bool GaussBlur(QImage& img)
{
    if(img.isNull())
        return false;

    int width = img.size().width();
    int height = img.size().height();

    const int ksz = 7;

    if (ksz > width || ksz > height)
        return false;

    SMatrix<double, ksz, ksz> kernel = {
      0.0, 0.0, 0.0, 0.00001, 0.0, 0.0, 0.0,
      0.0, 0.00005, 0.00107, 0.00291, 0.00107, 0.00005, 0.0,
      0.0, 0.00107, 0.02153, 0.05854, 0.02153, 0.00107, 0.0,
      0.00001, 0.00291, 0.05854, 0.15915, 0.05854, 0.00291, 0.00001,
      0.0, 0.00107, 0.02153, 0.05854, 0.02153, 0.00107, 0.0,
      0.0, 0.00005, 0.00107, 0.00291, 0.00107, 0.00005, 0.0,
      0.0, 0.0, 0.0, 0.00001, 0.0, 0.0, 0.0
    };

    Matrix<double> part_r(ksz, ksz);
    Matrix<double> part_g(ksz, ksz);
    Matrix<double> part_b(ksz, ksz);

    double div = std::accumulate(kernel.begin(), kernel.end(), 0.0);

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            for (int x = 0; x < ksz; x++)
            {
                for (int y = 0; y < ksz; y++)
                {
                    int posPixX = i - ksz / 2 + x;
                    int posPixY = j - ksz / 2 + y;

                    b_ctrl(posPixX, width);
                    b_ctrl(posPixY, height);

                    QRgb tmpc = img.pixel(posPixX, posPixY);

                    part_r[x][y] = qRed(tmpc);
                    part_g[x][y] = qGreen(tmpc);
                    part_b[x][y] = qBlue(tmpc);
                }
            }

            int r = inner_product(part_r.begin(), part_r.end(), kernel.begin(), 0.0) / div;
            int g = inner_product(part_g.begin(), part_g.end(), kernel.begin(), 0.0) / div;
            int b = inner_product(part_b.begin(), part_b.end(), kernel.begin(), 0.0) / div;

            img.setPixel(i, j, qRgb(r, g, b));
        }
    }

    return true;
}

Uint8 find_median(Matrix<Uint8>& m, array<Uint8, 256>& hist, bool ns)
{
    int ksz = m.size_dim1();

    if (ns)
    {
        hist.fill(0);

        for (const auto i : m)
            hist[i]++;
    }
    else {
        for (int i = 0; i < ksz; i++)
            hist[m[i][ksz - 1]]++;
    }

    int count = 0;
    Uint8 k = 0;
    for (; k < 256; k++)
    {
        count += hist[k];
        if (count > ksz*ksz / 2)
            break;
    }

    for (int i = 0; i < ksz; i++)
        hist[m[i][0]]--;

    return k;
}

bool MedianFunction(QImage& img, int ksz)
{
    //const int ksz = 7;
    int width = img.width();
    int height = img.height();

    if (ksz > width * height)
        return false;

    Matrix<Uint8> part_r(ksz, ksz);
    Matrix<Uint8> part_g(ksz, ksz);
    Matrix<Uint8> part_b(ksz, ksz);

    const int szg = 256;
    array<Uint8, szg> hist_r{ 0 };
    array<Uint8, szg> hist_g{ 0 };
    array<Uint8, szg> hist_b{ 0 };

    QImage new_img(width, height, QImage::Format_RGB32);

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            for (int x = 0; x < ksz; x++)
            {

                for (int y = 0; y < ksz; y++)
                {
                    int posPixX = i - ksz / 2 + x;
                    int posPixY = j - ksz / 2 + y;

                    b_ctrl(posPixX, width);
                    b_ctrl(posPixY, height);

                    QRgb curr = img.pixel(posPixX, posPixY);

                    part_r[x][y] = qRed(curr);
                    part_g[x][y] = qGreen(curr);
                    part_b[x][y] = qBlue(curr);

//                    Color curr = my_img.getPixel(posPixX, posPixY);

//                    part_r.at(x, y) = curr.r;
//                    part_g.at(x, y) = curr.g;
//                    part_b.at(x, y) = curr.b;
                }
            }

//            Color tmp;

            bool is_new_line = (j == 0);

            QRgb tmp = qRgb(find_median(part_r, hist_r, is_new_line),
                            find_median(part_g, hist_g, is_new_line),
                            find_median(part_b, hist_b, is_new_line)
                            );

//            tmp.r = find_median(part_r, hist_r, is_new_line);
//            tmp.g = find_median(part_g, hist_g, is_new_line);
//            tmp.b = find_median(part_b, hist_b, is_new_line);


//            new_img.setPixel(i, j, tmp);

            //img.setPixel(i, j, tmp);
            new_img.setPixel(i, j, tmp);
        }
    }

    img = std::move(new_img);

    return true;
}
*/
#endif // MYIMAGEPROC_H
