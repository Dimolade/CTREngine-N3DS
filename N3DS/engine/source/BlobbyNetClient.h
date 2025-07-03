#ifndef BLOBBYNETCLIENT_H
#define BLOBBYNETCLIENT_H

#include <string>
#include <3ds.h>
#include "BlobbyNetSocket.h"  // For the IP class

class NetClient {
public:
    static bool ConnectTo(const IP& target);
    static bool SendTo(const std::string& message);
    static std::string Receive();
    static bool CloseConnection();

private:
    static s32 clientSock;
    static bool connected;
};

#endif // NETCLIENT_H

