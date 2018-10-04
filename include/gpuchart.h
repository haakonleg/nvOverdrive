#ifndef GPUCHART_H
#define GPUCHART_H

#include <QtCharts>
#include <QChartView>
#include <memory>

class GPUChart : public QChartView {
public:
    GPUChart(QString title, int axisYSize, QWidget* parent = nullptr);

    void addValue(int value);
private:
    // Settings
    static const int CHART_SIZE = 300; // 300 seconds

    // Static members for defining the style of the chart
    static bool ST_INIT;
    static QPen PEN_STYLE;
    static QFont TITLE_FONT;
    static QFont LABELS_FONT;
    static QMargins MARGINS;

    std::unique_ptr<QChart> chart;
    QLineSeries* series;
    QGraphicsSimpleTextItem* currVal;
    int lastPosX = 0;
};

#endif // GPUCHART_H
