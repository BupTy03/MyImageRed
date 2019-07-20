#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "imageprocessing.hpp"

#include <QDialog>

class Histogram : public QDialog
{
    Q_OBJECT
public:
    explicit Histogram(
            const HistArray& Red,
            const HistArray& Green,
            const HistArray& Blue,
            QWidget* parent = nullptr);
};

#endif // HISTOGRAM_H
