#pragma once

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
The binary tree stracture is this:

                / (root node)

                    /User (level 1 node)
                    --------/User/Bob (leaf lvl1)
                    --------/User/Mark
                    --------/User/Kate
                        /User/Login (level 2 node)
                        --------/User/Bob/bob19066 (leaf lvl2)
                        --------/User/Mark/mark34355
                        --------/User/Kate/lol1111

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
 * std::unique_ptr, а все члены инициализируются осмысленным состоянием по
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