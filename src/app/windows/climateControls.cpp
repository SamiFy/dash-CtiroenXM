#include "app/windows/climateControls.hpp"

ClimateControlsWidget::ClimateControlsWidget(QWidget *parent)
    : FramedWidget(QColor(0,255,255), parent)
{
    // Set initial states
    m_blowerSpeed = 0;
    m_temperature = 21; // Default temperature

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    // 1. Title Label (from your original code)
    QLabel* titleLabel = new QLabel("Climate Controls");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 2. Blower Speed Control
    QHBoxLayout* blowerLayout = new QHBoxLayout();
    m_blowerMinusButton = new QPushButton("-");
    m_blowerPlusButton = new QPushButton("+");
    
    m_blowerSpeedBar = new QProgressBar();
    m_blowerSpeedBar->setRange(0, BLOWER_MAX);
    m_blowerSpeedBar->setTextVisible(false); // We only want the bars
    updateBlowerBar(); // Set initial value

    blowerLayout->addWidget(m_blowerMinusButton);
    blowerLayout->addWidget(m_blowerSpeedBar, 1); // Add stretch
    blowerLayout->addWidget(m_blowerPlusButton);
    mainLayout->addLayout(blowerLayout);

    // 3. Temperature Set Control
    QHBoxLayout* tempLayout = new QHBoxLayout();
    m_tempMinusButton = new QPushButton("-");
    m_tempPlusButton = new QPushButton("+");
    
    m_temperatureLabel = new QLabel();
    m_temperatureLabel->setObjectName("tempLabel"); // For styling
    m_temperatureLabel->setAlignment(Qt::AlignCenter);
    updateTemperatureLabel(); // Set initial text

    tempLayout->addWidget(m_tempMinusButton);
    tempLayout->addWidget(m_temperatureLabel, 1); // Add stretch
    tempLayout->addWidget(m_tempPlusButton);
    mainLayout->addLayout(tempLayout);

    // 4. Function Buttons
    // We use a container widget to apply specific styles
    QWidget* buttonContainer = new QWidget();
    buttonContainer->setObjectName("hvacButtonContainer");

    QHBoxLayout* buttonsLayout = new QHBoxLayout(buttonContainer);
    
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
    
    // (You would connect the checkable buttons to your main logic, e.g.)
    // connect(m_circulateButton, &QPushButton::toggled, this, &YourLogicClass::setCirculation);

    // 6. Apply Stylesheet
    // This QSS is critical for meeting your visual requirements.
    // The accent color is cyan (#00FFFF) to match your FramedWidget color.
    // this->setStyleSheet(R"(
    //     /* +/- buttons */
    //     QPushButton {
    //         font-size: 28px;
    //         font-weight: bold;
    //         min-width: 60px;
    //         min-height: 60px;
    //         background-color: #000;
    //         border: 1px solid #00cfa2;
    //     }
    //     QPushButton:pressed {
    //         background-color: #00cfa2; /* Cyan press */
    //         color: black;
    //     }

    //     /* Temperature Label */
    //     #tempLabel {
    //         font-size: 36px;
    //         font-weight: bold;
    //         color: white;
    //     }

    //     /* Blower Speed Bar (Req #1) */
    //     QProgressBar {
    //         border: 1px solid #00cfa2;
    //         border-radius: 5px;
    //         background-color: #000; /* Trough background */
    //         height: 40px;
    //     }
    //     QProgressBar::chunk {
    //         background-color: #00cfa2; /* Cyan 'lit' bars */
    //         width: 10px; /* This creates the 'bars' effect */
    //         margin: 2px;
    //     }

    //     /* HVAC Function Buttons (Req #3) */
    //     #hvacButtonContainer QPushButton {
    //         font-size: 16px;
    //         padding: 10px;
    //         min-height: 60px;
    //         background-color: transparent; /* OFF state: 'stroke only' */
    //         border: 2px solid #00cfa2;
    //         color: #00cfa2;
    //     }
    //     #hvacButtonContainer QPushButton:checked {
    //         background-color: #00cfa2; /* ON state: 'fill' */
    //         border: 2px solid #00cfa2;
    //         color: black; /* High contrast for 'on' state */
    //         font-weight: bold;
    //     }
    // )");
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
