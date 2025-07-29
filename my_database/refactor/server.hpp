#pragma once

#include "tree.hpp"      // Для доступа к структуре Node
#include <arpa/inet.h>  // для inet_addr()
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>  // для close()

#include <csignal>  // для signal()
#include <cstring>  // для strerror()
#include <iostream>

#define PORT 12002
#define HOST "127.0.0.1"

/**
 * @brief Запускает основной цикл сервера.
 * @details Инициализирует сервер, настраивает обработку дочерних процессов
 * и входит в бесконечный цикл приема новых клиентов.
 * @param root Указатель на корневой узел дерева данных.
 */
void run_server(std::shared_ptr<Node> root);
