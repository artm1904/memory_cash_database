#include "server.hpp"

bool server_continue;

Callback get_callback(std::string command) {
    Callback callback = nullptr;
    for (const auto &handler : commands_handlers) {
        if (handler.command == command) {
            callback = handler.callback;
            break;
        }
    }

    if (!callback) {
        return nullptr;
    }
    return callback;
}

/*------------------------------------HEADER_FUNCTION_IMPLEMENTATION--------------------------------------------*/
int init_server() {
    struct sockaddr_in sock;
    int sock_fd;

    sock.sin_family = AF_INET;  // Use Address Family: Ineternet - IPv4
    /*
     To conver in proper network byte order (little-endian vs. big-endian) use htons() and
     inet_addr() func
    */
    sock.sin_port = htons(PORT);
    sock.sin_addr.s_addr = inet_addr(HOST);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);  // SOCK_STREAM - use TCP

    if (sock_fd < 0) {
        std::cerr << "Error: Socket doesn't created" << std::endl;
        return -1;
    }

    if (bind(sock_fd, (struct sockaddr *)&sock, sizeof(sock)) != 0) {
        std::cerr << "Error: Bind failed" << std::endl;
        return -1;
    }

    if (listen(sock_fd, 20) != 0) {  // How many connections at a time
        std::cerr << "Error: Failed to prepare to accept connections on socket FD" << std::endl;
        return -1;
    }

    std::cout << "Server started listening on " << HOST << ":" << PORT << std::endl;

    return sock_fd;
}

void accept_connection(int sock_fd) {
    struct sockaddr_in client;
    int client_fd;

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

    std::thread(handle_connection, std::move(new_client)).detach();
}

void handle_connection(std::shared_ptr<Client> client) {
    char buffer[256];
    dprintf(client->client_fd, "100 Connected to server\n");
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(client->client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read <= 0) {
            std::cout << "Client " << client->ip << ":" << client->port << " disconnected."
                      << std::endl;
            break;
        }

        buffer[strcspn(buffer, "\r\n")] = 0;

        std::stringstream ss(buffer);
        std::string command, path, value;

        ss >> command >> path;

        std::getline(ss, value);

        if (!value.empty() && value.front() == ' ') {
            value.erase(0, 1);
        }

        std::cout << "  Command: '" << command << "', Path: '" << path << "', Value: '" << value
                  << "'" << std::endl;

        // dprintf(client->client_fd, "\n cmd:\t%s\n path:\t%s\n value:\t%s\n", command.c_str(),
        //         path.c_str(), value.c_str());

        // TODO: Здесь будет логика обработки команд и взаимодействия с деревом...
        Callback callback = get_callback(command);
        if (callback) {
            callback(client, path, value);
        } else {
            dprintf(client->client_fd, "Unknown command: %s\n", command.c_str());
        }
    }

    close(client->client_fd);
}

int handle_hello(std::shared_ptr<Client> client, std::string path, std::string value) {
    dprintf(client->client_fd, "Hello from server!\n");
    return 0;
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
