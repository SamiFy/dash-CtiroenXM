#include "app/windows/driveWindow.hpp"
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qpushbutton.h>

DriveWindowWidget::DriveWindowWidget(QWidget *parent)
    : FramedWidget(parent)
{
    m_rpm = 0;
    m_tacho = 0;

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel *citroenXM_Logo = new QLabel(this);
    QPixmap pixmap("./assets/graphics/CitroenXM-Logo.svg");
    if (!pixmap.isNull()) {
        citroenXM_Logo->setPixmap(pixmap.scaled(300, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        citroenXM_Logo->setText("Logo not found");
    }
    citroenXM_Logo->setStyleSheet("padding: 10px;");
    citroenXM_Logo->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(citroenXM_Logo, 0, Qt::AlignCenter);

    mainLayout->addStretch();
    
    QLabel* placeHolder_XManimation = new QLabel("Place Holder for Car interactive animation");
    placeHolder_XManimation->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(placeHolder_XManimation);

    QPushButton* testButton = new QPushButton("hello World!");
    mainLayout->addWidget(testButton);

}   
