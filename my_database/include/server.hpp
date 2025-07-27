#pragma once

#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // для inet_addr()
#include <unistd.h>  // для close()

#define PORT 12000
#define HOST "127.0.0.1"