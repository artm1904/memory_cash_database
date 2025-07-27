#include "server.hpp"

bool server_continue;

int server_loop() {
    struct sockaddr_in sock;
    int sock_fd;

    /*
     To conver in proper network byte order (little-endian vs. big-endian) use htons() and
     inet_addr() func
    */
    sock.sin_family = AF_INET;  // Use Address Family: Ineternet - IPv4
    sock.sin_port = htons(PORT);
    sock.sin_addr.s_addr = inet_addr(HOST);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);  // SOCK_STREAM - use TCP

    if (sock_fd > 0) {
        std::cerr << "Socket doesn't created" << std::endl;
        return -1;
    }

    if (bind(sock_fd, (struct sockaddr *)&sock, sizeof(sock)) != 0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    if (listen(sock_fd, 20) != 0) {  // How many connections at a time
        std::cerr << "Failed to prepare to accept connections on socket FD" << std::endl;
        return -1;
    }
}

int main(int argc, char const *argv[]) {
    server_continue = true;
    while (server_continue) {
    }

    return 0;
}