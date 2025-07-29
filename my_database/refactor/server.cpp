#include "server.hpp"

// Вспомогательная функция, видна только в этом файле
static int init_server() {
    struct sockaddr_in sock;
    int sock_fd;

    sock.sin_family = AF_INET;  // Use Address Family: Ineternet - IPv4
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

// Логика обработки одного клиента. Выполняется в дочернем процессе.
static void handle_connection(int client_fd, std::shared_ptr<Node> root) {
    (void)root;  // Пока не используем, но дерево доступно для будущей логики

    std::cout << "[Child " << getpid() << "] New connection handled." << std::endl;

    // 1. Отправляем приветственное сообщение
    const char *welcome_msg = "100 Connected to server\n";
    if (write(client_fd, welcome_msg, strlen(welcome_msg)) < 0) {
        std::cerr << "[Child " << getpid() << "] Error: Failed to write welcome message."
                  << std::endl;
        close(client_fd);
        exit(1);
    }

    // 2. Основной цикл обработки команд от клиента
    char buffer[1024] = {0};
    while (true) {
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "[Child " << getpid() << "] Received: " << buffer;
            // Здесь будет логика парсинга команд и взаимодействия с деревом
            const char *response = "200 OK\n";
            write(client_fd, response, strlen(response));
        } else {
            // Клиент отключился (bytes_read == 0) или произошла ошибка (bytes_read < 0)
            break;
        }
    }

    std::cout << "[Child " << getpid() << "] Connection closed." << std::endl;
    close(client_fd);
    exit(0);  // Завершаем дочерний процесс
}

void run_server(std::shared_ptr<Node> root) {
    int server_fd = init_server();
    if (server_fd < 0) {
        return;  // Ошибка уже выведена в init_server()
    }

    // *** РЕШЕНИЕ ПРОБЛЕМЫ ЗОМБИ-ПРОЦЕССОВ ***
    // Игнорируем сигнал SIGCHLD. Система сама будет убирать завершенные дочерние процессы.
    signal(SIGCHLD, SIG_IGN);

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            // EINTR может возникнуть, если accept прерван сигналом, это нормально
            if (errno == EINTR) continue;
            std::cerr << "Error: accept failed: " << strerror(errno) << std::endl;
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Error: fork failed: " << strerror(errno) << std::endl;
            close(client_fd);
        } else if (pid == 0) {  // Это дочерний процесс
            close(server_fd);   // Дочернему процессу не нужен слушающий сокет
            handle_connection(client_fd, root);
        } else {                // Это родительский процесс
            close(client_fd);   // Родительскому процессу не нужен сокет клиента
        }
    }
}

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;
    // Создаем демонстрационное дерево, как в вашем примере
    auto root = create_root_node();
    auto users_node = create_node(root, "Users");
    create_leaf(users_node, "bob", "bob_data");
    create_leaf(users_node, "kate", "kate_data");
    std::cout << "Data tree initialized." << std::endl;
    // Запускаем сервер, передав ему корень дерева
    run_server(root);
    return 0;
}