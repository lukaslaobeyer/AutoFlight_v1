#ifndef AUTOFLIGHT_BEBOPFTP_H
#define AUTOFLIGHT_BEBOPFTP_H

#define BEBOP_MEDIA_LOCATION "/internal_000/Bebop_Drone/media/"

#include <string>

namespace bebopftp
{
    void downloadMedia(std::string bebop_ip, std::string download_to, bool erase);
}


#endif
