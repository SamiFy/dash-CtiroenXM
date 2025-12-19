#include "app/windows/climateControls.hpp"
#include <qprogressbar.h>

ClimateControlsWidget::ClimateControlsWidget(QWidget *parent)
    : FramedWidget(parent)
{
    this->setMinimumWidth(350);
    this->setMaximumWidth(351);

    // Set initial states
    m_blowerSpeed = 0;
    m_temperature = 21; // Default temperature

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* blowerLayout = new QVBoxLayout();
    m_blowerMinusButton = new QPushButton("-");
    m_blowerPlusButton = new QPushButton("+");
    
    m_blowerSpeedBar = new QProgressBar;
    m_blowerSpeedBar->setRange(0, BLOWER_MAX);
    m_blowerSpeedBar->setTextVisible(false); // We only want the bars
    updateBlowerBar(); // Set initial value

    blowerLayout->addWidget(m_blowerPlusButton);
    blowerLayout->addWidget(m_blowerSpeedBar); // Add stretch
    blowerLayout->addWidget(m_blowerMinusButton);
    mainLayout->addLayout(blowerLayout);

    mainLayout->addStretch();

    // 3. Temperature Set Control
    QVBoxLayout* tempLayout = new QVBoxLayout();
    m_tempMinusButton = new QPushButton("-");
    m_tempPlusButton = new QPushButton("+");
    
    m_temperatureLabel = new QLabel();
    m_temperatureLabel->setObjectName("tempLabel"); // For styling
    m_temperatureLabel->setAlignment(Qt::AlignCenter);
    updateTemperatureLabel(); // Set initial text

    tempLayout->addWidget(m_tempPlusButton);
    tempLayout->addWidget(m_temperatureLabel, 1); // Add stretch
    tempLayout->addWidget(m_tempMinusButton);
    mainLayout->addLayout(tempLayout);

    mainLayout->addStretch();

    // 4. Function Buttons
    // We use a container widget to apply specific styles
    QWidget* buttonContainer = new QWidget();
    buttonContainer->setObjectName("hvacButtonContainer");

    QVBoxLayout* buttonsLayout = new QVBoxLayout(buttonContainer);
    
    m_circulateButton = new QPushButton("Circulate");
    m_circulateButton->setCheckable(true);

    m_flowButton = new QPushButton("Flow"); // "leg air or body air"
    m_flowButton->setCheckable(true);

    m_defogButton = new QPushButton("Defog");
    m_defogButton->setCheckable(true);

    m_rearHeatButton = new QPushButton("Rear Heat");
    m_rearHeatButton->setCheckable(true);

    buttonsLayout->addWidget(m_circulateButton);
    buttonsLayout->addWidget(m_flowButton);
    buttonsLayout->addWidget(m_defogButton);
    buttonsLayout->addWidget(m_rearHeatButton);
    
    mainLayout->addWidget(buttonContainer);
    
    // 5. Connect signals and slots
    connect(m_blowerPlusButton, &QPushButton::clicked, this, &ClimateControlsWidget::onBlowerPlus);
    connect(m_blowerMinusButton, &QPushButton::clicked, this, &ClimateControlsWidget::onBlowerMinus);
    connect(m_tempPlusButton, &QPushButton::clicked, this, &ClimateControlsWidget::onTempPlus);
    connect(m_tempMinusButton, &QPushButton::clicked, this, &ClimateControlsWidget::onTempMinus);
}

// --- Slot Implementations ---

void ClimateControlsWidget::onBlowerPlus()
{
    if (m_blowerSpeed < BLOWER_MAX) {
        m_blowerSpeed++;
        updateBlowerBar();
    }
}

void ClimateControlsWidget::onBlowerMinus()
{
    if (m_blowerSpeed > 0) {
        m_blowerSpeed--;
        updateBlowerBar();
    }
}

void ClimateControlsWidget::onTempPlus()
{
    if (m_temperature < TEMP_MAX) {
        m_temperature++;
        updateTemperatureLabel();
    }
}

void ClimateControlsWidget::onTempMinus()
{
    if (m_temperature > TEMP_MIN) {
        m_temperature--;
        updateTemperatureLabel();
    }
}

// --- Helper Implementations ---

void ClimateControlsWidget::updateTemperatureLabel()
{
    m_temperatureLabel->setText(QString::number(m_temperature) + " °C");
}

void ClimateControlsWidget::updateBlowerBar()
{
    m_blowerSpeedBar->setValue(m_blowerSpeed);
}
