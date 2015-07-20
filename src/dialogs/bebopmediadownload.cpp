#include <QtWidgets>

#include "bebopmediadownload.h"

BebopMediaDownload::BebopMediaDownload(std::string default_path, bool default_erase, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Download media from Bebop Drone"));

    setMinimumWidth(600);

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    QHBoxLayout *path_layout = new QHBoxLayout();
    QLabel *path_lbl = new QLabel(tr("File download location: "));
    path_layout->addWidget(path_lbl);
    path = new QLabel(QString::fromStdString(default_path));
    path_layout->addWidget(path);
    QPushButton *browse = new QPushButton(tr("Browse"));
    connect(browse, SIGNAL(clicked()), SLOT(browse()));
    path_layout->addWidget(browse);
    layout->addLayout(path_layout);

    erase = new QCheckBox(tr("Delete media on the Bebop's flash after download"));
    erase->setChecked(default_erase);
    layout->addWidget(erase);

    layout->addStretch(30);

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(bbox);

    QObject::connect(bbox, SIGNAL(accepted()), this, SLOT(handleAccept()));
    QObject::connect(bbox, SIGNAL(rejected()), this, SLOT(reject()));
}

void BebopMediaDownload::browse()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Find Files"), QDir::currentPath());

    if(!directory.isEmpty())
    {
        path->setText(directory);
    }
}

void BebopMediaDownload::handleAccept()
{
    _eraseOnDrone = erase->isChecked();
    _path = path->text().toStdString();

    accept();
}

std::string BebopMediaDownload::getSavePath()
{
    return _path;
}

bool BebopMediaDownload::eraseOnDrone()
{
    return _eraseOnDrone;
}