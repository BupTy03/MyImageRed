#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "imageprocessing.hpp"

#include <QDialog>

class Histogram : public QDialog
{
    Q_OBJECT
public:
    explicit Histogram(const HistRGB &histRGB,
            QWidget* parent = nullptr);
};

#endif // HISTOGRAM_H
