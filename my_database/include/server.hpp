#pragma once

#include <arpa/inet.h>  // For inet_addr()
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>  // For close()

#include <cstring>  // For memset
#include <iostream>
#include <memory>
#include <sstream>  // For std::stringstream
#include <string>   // For strcspn
#include <thread>   // For std::thread
#include <vector>

#define PORT 12004
// #define HOST "127.0.0.1"
#define HOST "192.168.31.28"

// A wrapper class for a client connection to ensure the socket is always closed.
class Client {
   public:
    // Takes ownership of the file descriptor.
    Client(int fd, std::string ip, int port) : fd_(fd), ip_(std::move(ip)), port_(port) {}

    // Destructor ensures the socket is closed when the Client object goes out of scope.
    ~Client() {
        if (fd_ >= 0) {
            close(fd_);
        }
    }

    // Disable copying to prevent double-closing the socket.
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    int get_fd() const { return fd_; }
    const std::string& get_ip() const { return ip_; }
    int get_port() const { return port_; }

    // Sends a string message to the client. Returns false on error.
    bool send(const std::string& message) const {
        if (write(fd_, message.c_str(), message.length()) < 0) {
            std::cerr << "Error writing to socket " << fd_ << ": " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

   private:
    int fd_;
    std::string ip_;
    int port_;
};

using Callback = int (*)(std::shared_ptr<Client> client, std::string path, std::string value);

struct s_command_handler {
    std::string command;
    Callback callback;
};

using CommandHandler = struct s_command_handler;

/**
 * @brief Инициализирует и настраивает TCP-сервер.
 *
 * @details Создает сокет, привязывает его к адресу (HOST) и порту (PORT),
 * а затем переводит в режим прослушивания входящих соединений.
 * @return Файловый дескриптор слушающего сокета в случае успеха, иначе -1.
 */
int init_server();

/**
 * @brief Принимает новое соединение и создает дочерний процесс для его обработки.
 * @details Блокируется до тех пор, пока не появится новое клиентское соединение.
 * После принятия создает дочерний процесс с помощью fork() для обработки клиента,
 * в то время как родительский процесс продолжает слушать новые соединения.
 * @param sock_fd Файловый дескриптор слушающего сокета.
 */
void accept_connection(int sock_fd);

/**
 * @brief Основной цикл обработки команд от одного клиента.
 * @details Эта функция выполняется в дочернем процессе. Она должна читать
 * команды из сокета клиента, обрабатывать их и отправлять ответы.
 * @param client Умный указатель на структуру с информацией о клиенте.
 */
void handle_connection(std::shared_ptr<Client> client);

int handle_hello(std::shared_ptr<Client> client, std::string path, std::string value);

std::vector<CommandHandler> commands_handlers = {{"hello", handle_hello}};