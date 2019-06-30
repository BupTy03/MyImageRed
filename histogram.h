#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "imageprocessing.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <array>

using namespace image_processing;

class Histogram : public QDialog
{
    Q_OBJECT
public:
    Histogram(const HistArray& Red,
              const HistArray& Green,
              const HistArray& Blue,
              QWidget* pwgt = nullptr);
};

#endif // HISTOGRAM_H
