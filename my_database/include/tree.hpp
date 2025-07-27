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
    std::shared_ptr<s_node> west;
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
