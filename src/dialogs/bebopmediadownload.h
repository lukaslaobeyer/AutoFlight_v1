#ifndef AUTOFLIGHT_BEBOPMEDIADOWNLOAD_H
#define AUTOFLIGHT_BEBOPMEDIADOWNLOAD_H

#include <QDialog>
#include <QWidget>
#include <QLabel>

class BebopMediaDownload : public QDialog
{
    Q_OBJECT

    public:
        explicit BebopMediaDownload(std::string default_path, bool default_erase, QWidget *parent = 0);

        std::string getSavePath();
        bool eraseOnDrone();

    private Q_SLOTS:
        void browse();
        void handleAccept();

    private:
        QLabel *path;
        QCheckBox *erase;

        bool _eraseOnDrone = false;
        std::string _path = "";
};

#endif
