#include "server.hpp"

#include <mutex>
/*-----------------------------------------STATIC_VARiABLES----------------------------------------------------*/

// Глобальный указатель на корень дерева и мьютекс для защиты доступа к нему.
static std::shared_ptr<Node> g_root;
static std::mutex g_tree_mutex;

/*-------------------------------------------HELPER_FUNCTION---------------------------------------------------*/

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
        std::cerr << "Error: Socket creation failed: " << strerror(errno) << std::endl;
        return -1;
    }

    if (bind(sock_fd, (struct sockaddr *)&sock, sizeof(sock)) != 0) {
        std::cerr << "Error: Bind failed for " << HOST << ":" << PORT << ": " << strerror(errno)
                  << std::endl;
        return -1;
    }

    if (listen(sock_fd, 20) != 0) {  // How many connections at a time
        std::cerr << "Error: Failed to listen on socket: " << strerror(errno) << std::endl;
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
    socklen_t len = sizeof(client);

    client_fd = accept(sock_fd, (struct sockaddr *)&client, &len);

    if (client_fd < 0) {
        return;
    }

    port = ntohs(client.sin_port);
    inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
    auto new_client = std::make_shared<Client>(client_fd, ip, port);

    std::cout << "New connection from " << ip << ":" << port << std::endl;

    std::thread(handle_connection, std::move(new_client)).detach();
}

void handle_connection(std::shared_ptr<Client> client) {
    char buffer[256];
    client->send("100 Connected to server\n");
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(client->get_fd(), buffer, sizeof(buffer) - 1);

        if (bytes_read <= 0) {
            std::cout << "Client " << client->get_ip() << ":" << client->get_port()
                      << " disconnected." << std::endl;
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

        Callback callback = get_callback(command);
        if (callback) {
            callback(client, path, value);
        } else {
            client->send("400 Bad Request: Unknown command '" + command + "'\n");
        }
    }
}

int handle_hello(std::shared_ptr<Client> client, std::string path, std::string value) {
    client->send("Hello from server!\n");
    return 0;
}

int handle_create_node(std::shared_ptr<Client> client, std::string path, std::string value) {
    if (path.empty()) {
        client->send("400 Bad Request: Path is required for CREATE_NODE.\n");
        return -1;
    }

    std::lock_guard<std::mutex> lock(g_tree_mutex);  // RAII Mutex
    if (auto new_node = create_node_by_path(g_root, path)) {
        client->send("200 OK: Node " + path + " created.\n");
    } else {
        client->send("500 Internal Server Error: Failed to create node " + path + ".\n");
    }
    return 0;
}

int handle_create_leaf(std::shared_ptr<Client> client, std::string path, std::string value) {
    if (path.empty()) {
        client->send("400 Bad Request: Path is required for CREATE_LEAF.\n");
        return -1;
    }

    std::lock_guard<std::mutex> lock(g_tree_mutex);
    if (auto new_leaf = create_leaf_by_path(g_root, path, value)) {
        client->send("200 OK: Leaf " + path + " created.\n");
    } else {
        client->send("500 Internal Server Error: Failed to create leaf " + path + ".\n");
    }
    return 0;
}

int handle_delete_node(std::shared_ptr<Client> client, std::string path, std::string value) {
    (void)value;
    if (path.empty() || path == "/") {
        client->send("400 Bad Request: Path is required and cannot be root for DELETE_NODE.\n");
        return -1;
    }

    std::lock_guard<std::mutex> lock(g_tree_mutex);
    if (delete_node_by_path_linear(g_root, path)) {
        client->send("200 OK: Node " + path + " deleted.\n");
    } else {
        client->send("404 Not Found: Failed to delete node " + path + ".\n");
    }
    return 0;
}

int handle_delete_leaf(std::shared_ptr<Client> client, std::string path, std::string value) {
    (void)value;
    if (path.empty()) {
        client->send("400 Bad Request: Path is required for DELETE_LEAF.\n");
        return -1;
    }

    std::lock_guard<std::mutex> lock(g_tree_mutex);
    if (delete_leaf_by_path_linear(g_root, path)) {
        client->send("200 OK: Leaf " + path + " deleted.\n");
    } else {
        client->send("404 Not Found: Failed to delete leaf " + path + ".\n");
    }
    return 0;
}

int handle_print_tree(std::shared_ptr<Client> client, std::string path, std::string value) {
    (void)value;
    if (path.empty()) {
        client->send("400 Bad Request: Path is required for PRINT_TREE.\n");
        return -1;
    }

    // Блокируем мьютекс на время чтения из дерева для потокобезопасности
    std::lock_guard<std::mutex> lock(g_tree_mutex);
    if (auto node = find_node_by_path_linear(g_root, path)) {
        client->send("200 OK\n" + print_tree_string(node));
    } else {
        client->send("404 Not Found: Node " + path + " not found.\n");
    }
    return 0;
}

std::vector<CommandHandler> commands_handlers = {{"hello", handle_hello},
                                                 {"CREATE_NODE", handle_create_node},
                                                 {"CREATE_LEAF", handle_create_leaf},
                                                 {"DELETE_NODE", handle_delete_node},
                                                 {"DELETE_LEAF", handle_delete_leaf},
                                                 {"PRINT_TREE", handle_print_tree}};

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;

    g_root = create_root_node();
    std::cout << "Data tree initialized." << std::endl;

    // Демонстрационное наполнение дерева
    create_node_by_path(g_root, "/Users");
    create_leaf_by_path(g_root, "/Users/readme", "This is a user directory.");

    int sock_fd = init_server();
    if (sock_fd < 0) {
        std::cerr << "Failed to init server" << std::endl;
        return -1;
    }
    while (true) {
        accept_connection(sock_fd);
    }

    std::cout << "Server stopped" << std::endl;
    close(sock_fd);
    return 0;
}




// Как можно улучшить функции `create_..._by_path`, чтобы они возвращали более конкретные коды ошибок, а не просто `nullptr`?