#ifndef NETSOCKET_H
#define NETSOCKET_H

#include <string>
#include <3ds.h>

class IP {
public:
    std::string ip;
    std::string port;
    bool success;

    IP() : ip(""), port(""), success(false) {}
    IP(const std::string& ipAddr, const std::string& portStr, bool ok = true)
        : ip(ipAddr), port(portStr), success(ok) {}
};

class NetSocket {
public:
    static IP StartServer(const std::string& port = "8080");
    static bool EndServer();

    static std::string ReceiveServer();
    static bool Send(const IP& sendTo, const std::string& message);

private:
    static s32 sock;
    static s32 csock;
    static u32* SOC_buffer;
    static bool initialized;
};

#endif // NETSOCKET_H
