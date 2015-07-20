#include "status.h"

#include "titledbox.h"

#include <QtWidgets>
#include <drones/bebop/types.h>

#include <memory>

Status::Status(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);

    setStyleSheet("padding: 0;");

        QVBoxLayout *contentLayout = new QVBoxLayout();
        contentLayout->setSpacing(2);
        contentLayout->setMargin(0);
        contentLayout->setContentsMargins(4, 4, 4, 4);

        QWidget *content = new QWidget();
        content->setLayout(contentLayout);

            status = new QLabel(tr("<qt><b>DISCONNECTED</b></qt>"));
            status->setAlignment(Qt::AlignCenter);
            status->setMargin(2);
            status->setStyleSheet("font: 12pt;");
            contentLayout->addWidget(status);

        TitledBox *box = new TitledBox(tr("Status"), content);
        layout->addWidget(box);

    QGraphicsDropShadowEffect *dropShadow = new QGraphicsDropShadowEffect();
    dropShadow->setBlurRadius(6);
    dropShadow->setColor(QColor(0, 0, 0));
    dropShadow->setOffset(0, 0);

    setGraphicsEffect(dropShadow);

    setLayout(layout);
}

void Status::statusUpdateAvailable(int s)
{
    if(s == drone::status::DISARMED)
    {
        status->setText(tr("<qt><b>DISARMED</b><br><small>press SHIFT+ALT+Y to arm</small></br></qt>"));
    }
    else if(s == drone::status::ARMED)
    {
        status->setText(tr("<qt><b>ARMED</b><br><small>press SHIFT+ALT+Y to disarm</small></br></qt>"));
    }
}