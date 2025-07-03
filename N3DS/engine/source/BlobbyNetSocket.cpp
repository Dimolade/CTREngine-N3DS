#include "BlobbyNetSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

s32 NetSocket::sock = -1;
s32 NetSocket::csock = -1;
u32* NetSocket::SOC_buffer = nullptr;
bool NetSocket::initialized = false;

IP NetSocket::StartServer(const std::string& portStr) {
    if (initialized) return IP("Already running", portStr, false);

    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if (!SOC_buffer) return IP("Memory error", portStr, false);

    if (socInit(SOC_buffer, SOC_BUFFERSIZE) != 0) {
        free(SOC_buffer);
        SOC_buffer = nullptr;
        return IP("socInit failed", portStr, false);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        socExit();
        return IP("Socket error", portStr, false);
    }

    sockaddr_in server = {};
    server.sin_family = AF_INET;
    server.sin_port = htons(std::stoi(portStr));
    server.sin_addr.s_addr = gethostid();

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) != 0) {
        close(sock);
        socExit();
        return IP("Bind failed", portStr, false);
    }

    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

    if (listen(sock, 1) != 0) {
        close(sock);
        socExit();
        return IP("Listen failed", portStr, false);
    }

    initialized = true;
    return IP(inet_ntoa(server.sin_addr), portStr, true);
}

bool NetSocket::EndServer() {
    if (csock > 0) close(csock);
    if (sock > 0) close(sock);
    if (SOC_buffer) free(SOC_buffer);
    socExit();

    sock = csock = -1;
    SOC_buffer = nullptr;
    initialized = false;
    return true;
}

std::string NetSocket::ReceiveServer() {
    sockaddr_in client = {};
    socklen_t clientlen = sizeof(client);
    char buffer[1024] = {};

    csock = accept(sock, (struct sockaddr*)&client, &clientlen);
    if (csock < 0) {
        return "";
    }

    fcntl(csock, F_SETFL, fcntl(csock, F_GETFL, 0) & ~O_NONBLOCK);
    int received = recv(csock, buffer, sizeof(buffer) - 1, 0);

    close(csock);
    csock = -1;

    if (received <= 0) return "";
    return std::string(buffer);
}

bool NetSocket::Send(const IP& sendTo, const std::string& message) {
    s32 clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock < 0) return false;

    sockaddr_in target = {};
    target.sin_family = AF_INET;
    target.sin_port = htons(std::stoi(sendTo.port));
    target.sin_addr.s_addr = inet_addr(sendTo.ip.c_str());

    if (connect(clientSock, (struct sockaddr*)&target, sizeof(target)) != 0) {
        close(clientSock);
        return false;
    }

    send(clientSock, message.c_str(), message.length(), 0);
    close(clientSock);
    return true;
}

