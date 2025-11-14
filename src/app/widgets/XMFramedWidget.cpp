#include "app/widgets/XMFramedWidget.hpp"
#include <QPainter>
#include <QPaintEvent>

FramedWidget::FramedWidget(const QColor& borderColor, QWidget* parent)
    : QWidget(parent),
      m_borderColor(borderColor)
{
    // This applies a glow effect, which gets applied to all children as well
    this->setAttribute(Qt::WA_TranslucentBackground); // Allow transparency
    auto* glowEffect = new QGraphicsDropShadowEffect(this);
    glowEffect->setBlurRadius(100);
    glowEffect->setOffset(0, 0);
    glowEffect->setColor(m_borderColor);

    this->setGraphicsEffect(glowEffect);
}

void FramedWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = this->rect();
    rect.adjust(1, 1, -1, -1); // Adjust for border thickness

    // // Draw background
    // painter.setBrush(Qt::red); // Or whatever background you want
    // painter.setPen(Qt::NoPen);
    // painter.drawRoundedRect(rect, 5, 5);

    // Draw border
    QPen pen(m_borderColor);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect);
}