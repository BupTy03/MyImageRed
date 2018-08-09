#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <array>

class Histogram : public QDialog
{
    Q_OBJECT
public:
    Histogram(const std::array<int, 256>& Red,
              const std::array<int, 256> &Green,
              const std::array<int, 256> &Blue,
              QWidget* pwgt = nullptr);
};

#endif // HISTOGRAM_H
