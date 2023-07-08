#ifndef MYWIDGET_H
#define MYWIDGET_H
#include <QtWidgets>

class MyWidget : public QWidget {
public:
    MyWidget(QWidget* parent = nullptr) : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QPainter painter(this);

        // 设置绘图参数
        int barWidth = 20;
        int maxValue = 100;
        int barSpacing = 30;
        int x = 50;
        int y = height() - 50;
        int chartHeight = y - 20;

        // 绘制坐标轴
        painter.drawLine(x, y, x, 20); // Y轴
        painter.drawLine(x, y, width() - 20, y); // X轴

        // 绘制柱状图
        QVector<int> data = {30, 60, 80, 50}; // 柱状图数据
        int numBars = data.size();
        int totalWidth = numBars * (barWidth + barSpacing) - barSpacing;
        int currentX = x + (width() - totalWidth) / 2;

        for (int i = 0; i < numBars; ++i) {
            int barHeight = (double)data[i] / maxValue * chartHeight;
            painter.drawRect(currentX, y - barHeight, barWidth, barHeight);
            currentX += barWidth + barSpacing;
        }
    }
};

#endif // MYWIDGET_H
