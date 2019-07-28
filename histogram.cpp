#include "histogram.hpp"

#include <QGridLayout>
#include <QLineSeries>
#include <QChartView>
#include <QValueAxis>

using namespace QtCharts;

Histogram::Histogram(const HistRGB& histRGB,
                     QWidget* parent)
    : QDialog{parent}
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(600, 400);
    setLayout(new QGridLayout());

    auto chartView = new QChartView(this);

    auto seriesRed = new QLineSeries();
    auto seriesGreen = new QLineSeries();
    auto seriesBlue = new QLineSeries();

    seriesRed->setColor(qRgb(255, 0, 0));
    seriesGreen->setColor(qRgb(0, 255, 0));
    seriesBlue->setColor(qRgb(0, 0, 255));

    Hist Red;
    Hist Green;
    Hist Blue;
    std::tie(Red, Green, Blue) = histRGB;
    assert(Red.size() == MAX_COLOR);
    assert(Green.size() == MAX_COLOR);
    assert(Blue.size() == MAX_COLOR);

    for(std::size_t i = 0; i < MAX_COLOR; ++i){
        (*seriesRed) << QPoint(static_cast<int>(i), static_cast<int>(Red.at(i)));
        (*seriesGreen) << QPoint(static_cast<int>(i), static_cast<int>(Green.at(i)));
        (*seriesBlue) << QPoint(static_cast<int>(i), static_cast<int>(Blue.at(i)));
    }


    auto chart = new QChart();
    chart->addSeries(seriesRed);
    chart->addSeries(seriesGreen);
    chart->addSeries(seriesBlue);

    (chart->legend())->hide();
    chart->setTitle("Гистограмма");


    auto axisX = new QValueAxis();
    axisX->setTitleText("интенсивность");
    axisX->setLabelFormat("%i");
    chart->addAxis(axisX, Qt::AlignBottom);
    seriesRed->attachAxis(axisX);

    auto axisY = new QValueAxis();
    axisY->setTitleText("кол-во пикселей");
    axisY->setLabelFormat("%i");
    chart->addAxis(axisY, Qt::AlignLeft);
    seriesRed->attachAxis(axisY);


    chartView->setChart(chart);
    (layout())->addWidget(chartView);
}
