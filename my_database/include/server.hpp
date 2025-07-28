#pragma once

#include <arpa/inet.h>  // для inet_addr()
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>  // для close()

#include <iostream>
#include <memory>
#include <string>

#define PORT 12002
#define HOST "127.0.0.1"

struct s_client {
    int client_fd;

    std::string ip;
    int port;
};

using Client = struct s_client;

int init_server();
void accept_connection(int sock_fd);
void child_loop(std::shared_ptr<Client> client);
