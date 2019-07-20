#ifndef MYCOLORITERATOR_H
#define MYCOLORITERATOR_H

#include <QImage>

#include <iterator>

using color_t = std::uint8_t;

class MyColorIterator : public std::iterator<std::input_iterator_tag, color_t>
{
private:
    MyColorIterator(QRgb* p) : p(p) {}
public:
    MyColorIterator(const MyColorIterator& it) :p(it.p) {}

    bool operator!=(MyColorIterator const& other) const { return p != other.p; }
    bool operator==(MyColorIterator const& other) const { return p == other.p; }
    QRgb& operator*(){ return *p; }
    long long operator-(MyColorIterator const& other){ return this->p - other.p; }
    MyColorIterator operator++() { p++; return *this; }
    MyColorIterator operator++(int) { auto tmp = *this; p++; return tmp; }

    static MyColorIterator Begin(QImage& img){ return MyColorIterator(reinterpret_cast<QRgb*>(img.bits())); }
    static MyColorIterator End(QImage& img) { return MyColorIterator(reinterpret_cast<QRgb*>(img.bits()) + img.width() * img.height()); }

    color_t red(){ return static_cast<color_t>(qRed(*p)); }
    color_t green() { return static_cast<color_t>(qGreen(*p)); }
    color_t blue() { return static_cast<color_t>(qBlue(*p)); }

private:
    QRgb * p{nullptr};
};

class ConstMyColorIterator : public std::iterator<std::input_iterator_tag, color_t>
{
private:
    ConstMyColorIterator(QRgb* p) : p(p) {}
public:
    ConstMyColorIterator(const ConstMyColorIterator& it) :p(it.p) {}

    bool operator!=(ConstMyColorIterator const& other) const { return p != other.p; }
    bool operator==(ConstMyColorIterator const& other) const { return p == other.p; }
    QRgb const& operator*(){ return *p; }
    long long operator-(ConstMyColorIterator const& other){ return this->p - other.p; }
    ConstMyColorIterator operator++() { p++; return *this; }
    ConstMyColorIterator operator++(int) { auto tmp = *this; p++; return tmp; }

    static ConstMyColorIterator CBegin(QImage& img){ return ConstMyColorIterator(reinterpret_cast<QRgb*>(img.bits())); }
    static ConstMyColorIterator CEnd(QImage& img) { return ConstMyColorIterator(reinterpret_cast<QRgb*>(img.bits()) + img.width() * img.height()); }

    color_t red(){ return static_cast<color_t>(qRed(*p)); }
    color_t green() { return static_cast<color_t>(qGreen(*p)); }
    color_t blue() { return static_cast<color_t>(qBlue(*p)); }
private:
    const QRgb* p{nullptr};
};

#endif // MYCOLORITERATOR_H