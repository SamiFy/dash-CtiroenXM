#include "app/windows/driveWindow.hpp"
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qpushbutton.h>

DriveWindowWidget::DriveWindowWidget(QWidget *parent)
    : FramedWidget(QColor(0,255,255), parent)
{
    m_rpm = 0;
    m_tacho = 0;

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QLabel* placeHolder_citroenXMLabel = new QLabel("Citroen XM");
    placeHolder_citroenXMLabel->setAlignment(Qt::AlignCenter);
    
    mainLayout->addWidget(placeHolder_citroenXMLabel);

    QLabel* placeHolder_XManimation = new QLabel("Place Holder for Car interactive animation");
    placeHolder_XManimation->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(placeHolder_XManimation);

    QPushButton* testButton = new QPushButton("hello World!");
    mainLayout->addWidget(testButton);

}   
