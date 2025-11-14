#ifndef CLIMATE_CONTROLS_HPP
#define CLIMATE_CONTROLS_HPP

#include <QtWidgets>
#include <app/widgets/XMFramedWidget.hpp>

class ClimateControlsWidget : public FramedWidget
{
    Q_OBJECT

public:
    // Change "QWidget" back to "FramedWidget" here too.
    explicit ClimateControlsWidget(QWidget *parent = nullptr);

private slots:
    // Slots to handle button clicks
    void onBlowerPlus();
    void onBlowerMinus();
    void onTempPlus();
    void onTempMinus();

private:
    // Helper functions to update UI
    void updateTemperatureLabel();
    void updateBlowerBar();

    // Blower Controls
    QPushButton* m_blowerMinusButton;
    QPushButton* m_blowerPlusButton;
    QProgressBar* m_blowerSpeedBar;

    // Temperature Controls
    QPushButton* m_tempMinusButton;
    QPushButton* m_tempPlusButton;
    QLabel* m_temperatureLabel;

    // Function Buttons
    QPushButton* m_circulateButton;
    QPushButton* m_flowButton;
    QPushButton* m_defogButton;
    QPushButton* m_rearHeatButton;

    // State variables
    int m_blowerSpeed;
    int m_temperature;

    // Constants
    static const int TEMP_MIN = 16;
    static const int TEMP_MAX = 30;
    static const int BLOWER_MAX = 10;
};

#endif // CLIMATE_CONTROLS_HPP