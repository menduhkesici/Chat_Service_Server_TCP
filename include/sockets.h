#pragma once

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class MasterSocket
{
private:
    int sock = 0;

public:
    MasterSocket(int __domain, int __type, int __protocol)
    {
        sock = socket(__domain, __type, __protocol);
        if (sock <= 0)
        {
            std::cerr << std::strerror(errno) << std::endl;
            throw std::runtime_error("socket failed!");
        }
    }

    ~MasterSocket()
    {
        shutdown(sock, SHUT_RDWR);
    }

    MasterSocket(const MasterSocket &) = delete;
    MasterSocket &operator=(const MasterSocket &) = delete;

    MasterSocket(MasterSocket &&) = delete;
    MasterSocket &operator=(MasterSocket &&) = delete;

    int getSock() { return sock; }
};

class ClientSocket
{
private:
    int sock = 0;
    std::string username;

public:
    ClientSocket(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len)
    {
        sock = accept(__fd, __addr, __addr_len);
        if (sock <= 0)
        {
            std::cerr << std::strerror(errno) << std::endl;
            throw std::runtime_error("accept failed!");
        }
    }

    ~ClientSocket()
    {
        close(sock);
    }

    ClientSocket(const ClientSocket &) = delete;
    ClientSocket &operator=(const ClientSocket &) = delete;

    ClientSocket(ClientSocket &&) = delete;
    ClientSocket &operator=(ClientSocket &&) = delete;

    int getSock() { return sock; }

    void sendMessage(const std::string& message)
    {
        if (send(sock, message.data(), message.length(), 0) != message.length())
            std::cerr << "send failed: " << std::strerror(errno) << std::endl;
    }

    void setUsername(const std::string& usrname)
    {
        username = usrname;
    }

    std::string getUsername()
    {
        return username;
    }

    bool usernameSelected()
    {
        return (username.length() > 0);
    }
};
