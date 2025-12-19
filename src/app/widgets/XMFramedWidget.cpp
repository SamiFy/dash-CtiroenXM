#include "app/widgets/XMFramedWidget.hpp"
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPaintEvent>
#include <qcolor.h>
#include <qcoreevent.h>
#include <qgraphicseffect.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qpalette.h>
#include <qwidget.h>

ThemeAwareGlow::ThemeAwareGlow(QWidget *parent){
    setOffset(0,0);
    setBlurRadius(30);
    updateColorFromPalette();
}

bool ThemeAwareGlow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
        updateColorFromPalette();
    }
    return QGraphicsDropShadowEffect::eventFilter(obj, event);
}

void ThemeAwareGlow::updateColorFromPalette() {
    setColor(QColor(255, 220, 0, 255));

    if (parent()) {
        QWidget *w = qobject_cast<QWidget*>(parent());
        if (w) {
            QColor baseColor = w->palette().color(QPalette::Window);
            setColor(baseColor);
        }
    }
}

FramedWidget::FramedWidget(QWidget* parent)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);

    // ThemeAwareGlow* glow = new ThemeAwareGlow(this);
    // this->setGraphicsEffect(glow);
}

void FramedWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = this->rect();
    rect.adjust(1, 1, -1, -1); // Adjust for border thickness

    // Draw border
    QPen pen(palette().color(QPalette::Base));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect);
}