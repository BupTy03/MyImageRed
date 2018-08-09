#include "histogram.h"

#include <QLineSeries>
#include <QChartView>
#include <QValueAxis>
#include <QDebug>

using namespace QtCharts;

Histogram::Histogram(const std::array<int, 256>& Red,
                     const std::array<int, 256>& Green,
                     const std::array<int, 256>& Blue,
                     QWidget *pwgt) : QDialog(pwgt)
{
    this->setMinimumSize(600, 400);
    this->setLayout(new QGridLayout());

    // Создаём представление графика
    QChartView *chartView = new QChartView(this);

    QLineSeries* seriesRed = new QLineSeries();
    QLineSeries* seriesGreen = new QLineSeries();
    QLineSeries* seriesBlue = new QLineSeries();

    seriesRed->setColor(qRgb(255, 0, 0));
    seriesGreen->setColor(qRgb(0, 255, 0));
    seriesBlue->setColor(qRgb(0, 0, 255));

    for(int i = 0; i < 256; ++i){
        *seriesRed << QPoint(i, Red[i]);
        *seriesGreen << QPoint(i, Green[i]);
        *seriesBlue << QPoint(i, Blue[i]);
    }

    // Создаём график
    QChart *chart = new QChart();
    chart->addSeries(seriesRed);
    chart->addSeries(seriesGreen);
    chart->addSeries(seriesBlue);

    chart->legend()->hide();
    chart->setTitle("Гистограмма");

    // Настройка осей графика
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("интенсивность");
    axisX->setLabelFormat("%i");
    chart->addAxis(axisX, Qt::AlignBottom);
    seriesRed->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("кол-во пикселей");
    axisY->setLabelFormat("%i");
    chart->addAxis(axisY, Qt::AlignLeft);
    seriesRed->attachAxis(axisY);

    // Устанавливаем график в представление
    chartView->setChart(chart);

    this->layout()->addWidget(chartView);

}
