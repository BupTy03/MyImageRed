#include "histogram.hpp"

#include <QGridLayout>
#include <QLineSeries>
#include <QChartView>
#include <QValueAxis>

using namespace QtCharts;

Histogram::Histogram(const HistArray& Red,
                     const HistArray& Green,
                     const HistArray& Blue,
                     QWidget* parent)
    : QDialog{parent}
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(600, 400);
    setLayout(new QGridLayout());

    QChartView* chartView = new QChartView(this);

    QLineSeries* seriesRed = new QLineSeries();
    QLineSeries* seriesGreen = new QLineSeries();
    QLineSeries* seriesBlue = new QLineSeries();

    seriesRed->setColor(qRgb(255, 0, 0));
    seriesGreen->setColor(qRgb(0, 255, 0));
    seriesBlue->setColor(qRgb(0, 0, 255));

    for(std::size_t i = 0; i < MAX_COLOR; ++i){
        (*seriesRed) << QPoint(static_cast<int>(i), static_cast<int>(Red[i]));
        (*seriesGreen) << QPoint(static_cast<int>(i), static_cast<int>(Green[i]));
        (*seriesBlue) << QPoint(static_cast<int>(i), static_cast<int>(Blue[i]));
    }


    QChart* chart = new QChart();
    chart->addSeries(seriesRed);
    chart->addSeries(seriesGreen);
    chart->addSeries(seriesBlue);

    (chart->legend())->hide();
    chart->setTitle("Гистограмма");


    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("интенсивность");
    axisX->setLabelFormat("%i");
    chart->addAxis(axisX, Qt::AlignBottom);
    seriesRed->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("кол-во пикселей");
    axisY->setLabelFormat("%i");
    chart->addAxis(axisY, Qt::AlignLeft);
    seriesRed->attachAxis(axisY);


    chartView->setChart(chart);
    (layout())->addWidget(chartView);
}
