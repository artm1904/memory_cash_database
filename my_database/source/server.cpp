#include "server.hpp"

bool server_continue;
bool child_continue;

int init_server() {
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

    if (sock_fd < 0) {
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

    std::cout << "Server started listening on " << HOST << ":" << PORT << std::endl;

    return sock_fd;
}

void accept_connection(int sock_fd) {
    struct sockaddr_in client;
    int client_fd;

    pid_t pid;

    char ip[16];
    int port;
    int len = sizeof(client);

    client_fd = accept(sock_fd, (struct sockaddr *)&client, (socklen_t *)&len);

    if (client_fd < 0) {
        return;
    }

    port = ntohs(client.sin_port);
    inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
    auto new_client = std::make_shared<Client>();
    new_client->client_fd = client_fd;
    new_client->port = port;
    new_client->ip = ip;

    std::cout << "New connection from " << ip << ":" << port << std::endl;

    pid = fork();
    if (pid != 0) {
        return;
    } else {
        child_continue = true;
        dprintf(client_fd, "100 Connected to server\n");

        while (child_continue) {
            child_loop(new_client);
        }

        close(client_fd);
        return;
    }
}

void child_loop(std::shared_ptr<Client> client) {
    sleep(1000);
    child_continue = false;
    return;
}

int main(int argc, char const *argv[]) {
    int sock_fd;
    server_continue = true;
    sock_fd = init_server();
    if (sock_fd < 0) {
        std::cerr << "Failed to init server" << std::endl;
        return -1;
    }
    while (server_continue) {
        accept_connection(sock_fd);
    }

    std::cout << "Server stopped" << std::endl;
    close(sock_fd);
    return 0;
}