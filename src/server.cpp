// Chat service server on TCP/IP connection
// Example code used: https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

#include "../include/server.h"
#include "../include/sockets.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <memory>

const int PORT = 8888;

inline std::string trimString(char *str);
inline std::string trimStringInclNewline(char *str);
inline std::string colorString(const std::string &str);
inline bool existsClientWithUsername(const std::vector<std::unique_ptr<ClientSocket>> &clients, const std::string &username);

const std::string clear_line = "\33[2K\r";
const int message_limit = 1024;

void server()
{
    char buffer[message_limit + 2]; // data buffer

    // set of socket descriptors
    fd_set readfds;

    // a message
    std::string message;

    // create a master socket
    MasterSocket master_socket(AF_INET, SOCK_STREAM, 0);

    // set master socket to allow multiple connections,
    // this is just a good habit, it will work without this
    int opt = 1;
    if (setsockopt(master_socket.getSock(), SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        std::cerr << std::strerror(errno) << std::endl;
        throw std::runtime_error("setsockopt failed!");
    }

    // type of socket created
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind the socket to localhost port 8888
    if (bind(master_socket.getSock(), (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << std::strerror(errno) << std::endl;
        throw std::runtime_error("bind failed!");
    }

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket.getSock(), 3) < 0)
    {
        std::cerr << std::strerror(errno) << std::endl;
        throw std::runtime_error("listen failed!");
    }
    std::cout << "Server started" << std::endl;
    std::cout << "Type 'telnet localhost " << PORT << "' on client terminal to connect to the server" << std::endl;
    std::cout << "Press '^C' to close the server" << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    int addrlen = sizeof(address);
    std::vector<std::unique_ptr<ClientSocket>> clients;

    // accept the incoming connection
    while (true)
    {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(master_socket.getSock(), &readfds);
        int max_sd = master_socket.getSock();

        // add child sockets to set
        for (const auto &client : clients)
        {
            // socket descriptor
            int sd = client->getSock();

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // wait for an activity on one of the sockets, timeout is infinite
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
        {
            std::cerr << "select error" << std::endl;
        }

        // If something happened on the master socket, then its an incoming connection
        if (FD_ISSET(master_socket.getSock(), &readfds))
        {
            auto new_socket = std::make_unique<ClientSocket>(
                master_socket.getSock(), (struct sockaddr *)&address, (socklen_t *)&addrlen);

            // inform user of socket number - used in send and receive commands
            std::cout << "Client connected: Socket fd: " << new_socket->getSock() << ", IP: "
                      << inet_ntoa(address.sin_addr) << ", Port: " << ntohs(address.sin_port) << std::endl;

            // send new connection greeting message
            message = "Hello Client, welcome to the chat service.\n";
            new_socket->sendMessage(message);

            bool userExists = false;
            message = "People in the chat lobby:";
            for (const auto &client : clients)
                if (client->usernameSelected())
                {
                    userExists = true;
                    message += " '" + colorString(client->getUsername()) + "'";
                }
            message += ".";
            if (!userExists)
                message = "There are no people in the chat lobby.";
            message += "\nPlease type in your username and press Enter: ";
            new_socket->sendMessage(message);

            // add new socket to array of sockets
            clients.push_back(std::move(new_socket));
        }

        // else its some IO operation on some other socket
        for (auto itClient = clients.begin(); itClient != clients.end(); itClient++)
        {
            int sd = (*itClient)->getSock();

            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing, and also read the incoming message
                int valread = recv(sd, buffer, message_limit, 0);
                if (valread == 0)
                {
                    // Somebody disconnected, get his details and print
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    std::cout << "Client disconnected, IP: " << inet_ntoa(address.sin_addr)
                              << ", Port: " << ntohs(address.sin_port) << std::endl;

                    // Close the socket and delete from vector
                    bool username_selected = (*itClient)->usernameSelected();
                    std::string username = (*itClient)->getUsername();
                    clients.erase(itClient--);

                    if (username_selected)
                    {
                        message = clear_line + colorString(username) + " left the chat\n";
                        for (const auto &client : clients)
                            if (client->usernameSelected())
                                client->sendMessage(message);
                        std::cout << message;
                    }
                }

                // Echo back the message that came in
                else
                {
                    if (!(*itClient)->usernameSelected())
                    {
                        // Set the username
                        buffer[valread - 2] = '\0';
                        std::string tmpUsername = trimStringInclNewline(buffer);

                        if (tmpUsername.length() == 0)
                        {
                            message = "Username cannot be blank! Please choose another username: ";
                            (*itClient)->sendMessage(message);
                        }
                        else if (existsClientWithUsername(clients, tmpUsername))
                        {
                            message = "Username already exists! Please choose another username: ";
                            (*itClient)->sendMessage(message);
                        }
                        else
                        {
                            message = clear_line + colorString(tmpUsername) + " entered the chat\n";
                            for (const auto &client : clients)
                                if (client->usernameSelected())
                                    client->sendMessage(message);
                            std::cout << message;

                            (*itClient)->setUsername(tmpUsername);

                            message = "You have entered the chat lobby. Start typing\n";
                            (*itClient)->sendMessage(message);
                        }
                    }
                    else if (valread > 2)
                    {
                        if (buffer[valread - 1] == '\n')
                        {
                            buffer[valread] = '\0';
                        }
                        else
                        {
                            buffer[valread + 1] = '\0';
                            buffer[valread] = '\n';
                        }
                        message = clear_line + colorString((*itClient)->getUsername() + ": ") + trimString(buffer);
                        for (const auto &client : clients)
                            if (client->usernameSelected())
                                client->sendMessage(message);
                    }
                }
            }
        }
    }
}

inline std::string trimString(char *str)
{
    int j = 0;
    for (int i = 0; ((i < message_limit - 1)) && (str[i] != '\0'); ++i)
        if ((str[i] >= ' ') || (str[i] == '\n'))
            str[j++] = str[i];
    str[j] = '\0';
    return std::string(str, j + 1);
}

inline std::string trimStringInclNewline(char *str)
{
    int j = 0;
    for (int i = 0; ((i < message_limit - 1)) && (str[i] != '\0'); ++i)
        if (str[i] >= ' ')
            str[j++] = str[i];
    str[j] = '\0';
    return std::string(str, j + 1);
}

inline std::string colorString(const std::string &str)
{
    return "\033[1;31m" + str + "\033[0m";
}

inline bool existsClientWithUsername(const std::vector<std::unique_ptr<ClientSocket>> &clients, const std::string &username)
{
    auto iter = std::find_if(clients.begin(), clients.end(), [&](const std::unique_ptr<ClientSocket> &ts)
                             { return ts->getUsername() == username; });
    return iter != clients.end();
}
