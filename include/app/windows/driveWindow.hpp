#ifndef DRIVE_WINDOW_HPP
#define DRIVE_WINDOW_HPP

#include <QtWidgets>
#include <app/widgets/XMFramedWidget.hpp>
#include <qlabel.h>
#include <qwidget.h>

class DriveWindowWidget : public FramedWidget
{
    Q_OBJECT

    public:
        explicit DriveWindowWidget(QWidget *parent = nullptr);

    private slots: // prototypes
        // void onGearChange();
        // void onSpeedChange();

    private:
        void updateTacho();
        void updateRPM();
        void updateGear();
        void updateTransmission();

    QLabel* m_tachometerLabel;
    QLabel* m_rpmLabel;
    QLabel* m_gearLabel;
    QLabel* m_transmissionLabel;

    int m_tacho;
    int m_rpm;
    
};

#endif // DRIVE_WINDOW_HPP