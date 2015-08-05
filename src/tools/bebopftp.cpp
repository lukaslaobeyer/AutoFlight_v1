#include "bebopftp.h"

#include <ftp/FTPClient.h>
#include <ftp/FTPDataTypes.h>
#include <ftp/FTPFileStatus.h>

#include <iostream>

using namespace bebopftp;

void bebopftp::downloadMedia(std::string bebop_ip, std::string download_to, bool erase)
{
    if(download_to.back() != '/' && download_to.back() != '\\')
    {
        download_to.append("/");
    }

    nsFTP::CFTPClient ftpClient;
    nsFTP::CLogonInfo logonInfo(bebop_ip);

    ftpClient.Login(logonInfo);
    nsFTP::TFTPFileStatusShPtrVec list;
    ftpClient.List(BEBOP_MEDIA_LOCATION, list);

    for(nsFTP::TFTPFileStatusShPtrVec::iterator it=list.begin(); it!=list.end(); it++)
    {
        std::cout << "Downloading " << (*it)->Name() << " into " << download_to << std::endl;
        ftpClient.DownloadFile(BEBOP_MEDIA_LOCATION + (*it)->Name(), download_to + (*it)->Name());
        if(erase)
        {
            ftpClient.Delete(BEBOP_MEDIA_LOCATION + (*it)->Name());
        }
    }

    ftpClient.Logout();

    std::cout << "Done." << std::endl;
}