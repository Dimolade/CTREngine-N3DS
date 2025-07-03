#include "BlobbyNetClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

s32 NetClient::clientSock = -1;
bool NetClient::connected = false;

bool NetClient::ConnectTo(const IP& target) {
    if (!target.success) return false;

    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock < 0) return false;

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::stoi(target.port));
    addr.sin_addr.s_addr = inet_addr(target.ip.c_str());

    if (connect(clientSock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(clientSock);
        clientSock = -1;
        return false;
    }

    connected = true;
    return true;
}

bool NetClient::SendTo(const std::string& message) {
    if (!connected || clientSock < 0) return false;

    int sent = send(clientSock, message.c_str(), message.length(), 0);
    return sent >= 0;
}

std::string NetClient::Receive() {
    if (!connected || clientSock < 0) return "";

    char buffer[1024] = {};
    int received = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) return "";

    return std::string(buffer);
}

bool NetClient::CloseConnection() {
    if (clientSock >= 0) {
        close(clientSock);
        clientSock = -1;
        connected = false;
        return true;
    }
    return false;
}
