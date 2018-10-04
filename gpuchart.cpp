#include "gpuchart.h"

bool GPUChart::ST_INIT = false;
QPen GPUChart::PEN_STYLE;
QFont GPUChart::TITLE_FONT;
QFont GPUChart::LABELS_FONT;
QMargins GPUChart::MARGINS;

GPUChart::GPUChart(QString title, int axisYSize, QWidget* parent) : QChartView(parent) {
    if (!ST_INIT) {
        PEN_STYLE.setColor(Qt::red);
        PEN_STYLE.setWidth(1);
        TITLE_FONT.setPixelSize(12);
        LABELS_FONT.setPixelSize(12);
        MARGINS.setBottom(0);
        MARGINS.setLeft(2);
        MARGINS.setRight(2);
        MARGINS.setTop(0);
        ST_INIT = true;
    }

    setMaximumHeight(120);

    series = new QLineSeries(this);
    series->setPen(PEN_STYLE);

    // Chart
    chart = std::make_unique<QChart>();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setTitle(title);
    chart->setTitleFont(TITLE_FONT);
    chart->setMargins(MARGINS);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0);

    // Axis
    QValueAxis* axisX = new QValueAxis(chart.get());
    QValueAxis* axisY = new QValueAxis(chart.get());
    axisX->setRange(0, CHART_SIZE + (CHART_SIZE / 10));
    axisX->setLabelsVisible(false);
    axisY->setLabelsFont(LABELS_FONT);
    axisY->setRange(0, axisYSize);
    axisY->setLabelFormat("%.0f");
    chart->setAxisX(axisX, series);
    chart->setAxisY(axisY, series);

    currVal = new QGraphicsSimpleTextItem(chart.get());
    currVal->setFont(LABELS_FONT);

    setRenderHint(QPainter::Antialiasing);
    setChart(chart.get());
}

void GPUChart::addValue(int value) {
    series->append(lastPosX++, value);
    if (series->count() == CHART_SIZE) {
        series->remove(0);
        chart->scroll(chart->plotArea().width() / (CHART_SIZE + (CHART_SIZE / 10)), 0);
    }

    QPointF lastPos = chart->mapToPosition(series->at(series->count()-1));
    currVal->setPos(lastPos.x() + 5, lastPos.y() - 10);
    currVal->setText(QString::number(value));
}
