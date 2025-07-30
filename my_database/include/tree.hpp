#pragma once

#include <algorithm>  // For std::remove
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

enum class Tag : unsigned char {
    Root = 1, /* 00 01*/
    Node = 2, /* 00 10*/
    Leaf = 4, /* 01 00*/
};

inline constexpr Tag operator|(Tag a, Tag b) {
    return static_cast<Tag>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
}

/*
The common view tree stracture is this:

                / (root node)
                ├── /Users
                │   ├── /Users/Login
                │   │   ├── /Users/Login/bob (лист)
                │   │   └── /Users/Login/kate (лист)
                │   └── /Users/Password
                └── /Shops

*/

// Forward declarations to resolve circular dependency between Node and Leaf
struct s_node;
struct s_leaf;
using Node = struct s_node;
using Leaf = struct s_leaf;

struct s_node {
    Tag tag;
    std::weak_ptr<s_node> parent;  // To prevent cycles of owning
    std::vector<std::shared_ptr<s_node>> childs;
    std::shared_ptr<s_leaf> east;
    std::string path;
};

struct s_leaf {
    Tag tag;
    std::variant<std::weak_ptr<s_node>, std::weak_ptr<s_leaf>> parent;
    std::shared_ptr<s_leaf> west;
    std::shared_ptr<s_leaf> east;
    std::string path;

    std::string value;
};

/**
 * @brief Создает корневой узел для дерева.
 *
 * Эта фабричная функция создает корневой узел, по указанному пути. Она
 * гарантирует, что узел выделяется в куче и управляется std::shared_ptr, а все
 * его члены инициализируются осмысленным состоянием по умолчанию.
 *
 * @return std::shared_ptr<Node>, владеющий указатель на созданный узел.
 */
std::shared_ptr<Node> create_root_node();

/**
 * @brief Создает новый узел для дерева.
 *
 * Эта фабричная функция создает новый узел оносительно корневого, по
 * указанному пути. Она гарантирует, что узел выделяется в куче и управляется
 * std::shared_ptr, а все члены инициализируются осмысленным состоянием по
 * умолчанию.
 *
 * @param parent Родительский узел.
 * @param path Путь для нового узла.
 * @return std::shared_ptr<Node>, владеющий указатель на новый узел.
 */
std::shared_ptr<Node> create_node(const std::shared_ptr<Node> &parent, std::string path);

/**
 * @brief Находит последний лист в двухсвязанном списке, начинающемся с
 * parent->east.
 * @param parent Родительский узел, чьи листья нужно проверить.
 * @return std::shared_ptr<Leaf> на последний лист или nullptr, если листьев
 * нет.
 */
std::shared_ptr<Leaf> find_last_linear(const std::shared_ptr<Node> &parent);

/**
 * @brief Создает новый лист (файл) и присоединяет его к родительскому узлу.
 *
 * @param parent Родительский узел (каталог), к которому добавляется лист.
 * @param path Имя листа (файла).
 * @param value Данные, которые будет хранить лист.
 * @return std::shared_ptr<Leaf>, владеющий указатель на новый лист.
 */
std::shared_ptr<Leaf> create_leaf(const std::shared_ptr<Node> &parent, std::string path,
                                  std::string value);

void print_tree(const std::shared_ptr<Node> &root);

std::shared_ptr<Node> find_node_by_path_linear(const std::shared_ptr<Node> &root,
                                               const std::string &path);

/**
 * @brief Удаляет узел из дерева по его полному пути.
 *
 * Эта функция находит узел по его пути и удаляет его из списка
 * дочерних элементов родителя. Узел и все его поддерево будут автоматически
 * освобождены благодаря умным указателям, если на них не будет внешних ссылок.
 *
 * @param root Корневой узел дерева.
 * @param path Полный путь удаляемого узла (например, "/Users/Login").
 * @return true, если узел был найден и удален, иначе false.
 */
bool delete_node_by_path_linear(const std::shared_ptr<Node> &root, const std::string &path);

/**
 * @brief Находит лист в дереве по его полному пути.
 *
 * @param root Корневой узел дерева для начала поиска.
 * @param path Полный путь к листу (например, "/Users/Login/bob").
 * @return std::shared_ptr<Leaf> на найденный лист или nullptr, если лист не найден.
 */
std::shared_ptr<Leaf> find_leaf_by_path_linear(const std::shared_ptr<Node> &root,
                                               const std::string &path);

/**
 * @brief Удаляет лист из дерева по его полному пути.
 *
 * @param root Корневой узел дерева.
 * @param path Полный путь удаляемого листа (например, "/Users/Login/bob").
 * @return true, если лист был найден и удален, иначе false.
 */
bool delete_leaf_by_path_linear(const std::shared_ptr<Node> &root, const std::string &path);

/**
 * @brief Создает узел по полному пути, если это возможно.
 *
 * Проверяет, что родительский узел существует и что узел с таким путем еще не создан.
 * Может создать новый узел, если родительский уже создан.
 * Пример: в общем дереве есть только /Users, но мы хотим добавить /Users/Data/Password. Нужно два
 * раза выполнить создание узла, сначала /Users/Data, потом /Users/Data/Password
 *
 * @param root Корневой узел дерева.
 * @param path Полный путь для нового узла (например, "/Users/NewFolder").
 * @return std::shared_ptr<Node> на созданный узел или nullptr в случае ошибки.
 */
std::shared_ptr<Node> create_node_by_path(const std::shared_ptr<Node> &root,
                                          const std::string &path);

/**
 * @brief Создает лист по полному пути, если это возможно.
 *
 * Проверяет, что родительский узел существует и что узел/лист с таким путем еще не создан.
 *
 * @param root Корневой узел дерева.
 * @param path Полный путь для нового листа (например, "/Users/Login/new_user").
 * @param value Значение, которое будет хранить лист.
 * @return std::shared_ptr<Leaf> на созданный лист или nullptr в случае ошибки.
 */
std::shared_ptr<Leaf> create_leaf_by_path(const std::shared_ptr<Node> &root,
                                          const std::string &path, const std::string &value);
