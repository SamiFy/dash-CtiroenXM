#pragma once

#include <QWidget>
#include <QColor>
#include <QGraphicsDropShadowEffect>

class FramedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FramedWidget(const QColor& borderColor = QColor(0, 255, 255), 
                          QWidget* parent = nullptr);

private:
    QColor m_borderColor;

    void paintEvent(QPaintEvent* event);
};