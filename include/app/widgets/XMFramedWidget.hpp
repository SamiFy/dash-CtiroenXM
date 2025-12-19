#pragma once

#include <QWidget>
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <qcoreevent.h>
#include <qgraphicseffect.h>
#include <qwidget.h>

class ThemeAwareGlow : public QGraphicsDropShadowEffect {
   Q_OBJECT
   
    public:
        explicit ThemeAwareGlow(QWidget* parent = nullptr);

    protected:
        bool eventFilter(QObject *obj, QEvent *event);

    private:
        void updateColorFromPalette();

};

class FramedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FramedWidget(QWidget* parent = nullptr);

private:
    void paintEvent(QPaintEvent* event);
};