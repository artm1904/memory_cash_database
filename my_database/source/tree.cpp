#include "tree.hpp"


std::shared_ptr<Node> create_root_node() {
    auto root = std::make_shared<Node>();
    root->tag = Tag::Root | Tag::Node;
    root->path = "/";
    // Дочерние указатели по умолчанию равны nullptr в std::unique_ptr
    return root;
}


std::shared_ptr<Node> create_node(const std::shared_ptr<Node> &parent, std::string path) {
    assert(parent != nullptr && "Parent node cannot be null");
    assert(!path.empty() && "Node path cannot be empty");

    auto new_node = std::make_shared<Node>();
    new_node->tag = Tag::Node;
    new_node->path = std::move(path);
    new_node->parent = parent;

    parent->west = new_node;
    return new_node;
}


std::shared_ptr<Leaf> find_last_linear(const std::shared_ptr<Node> &parent) {
    assert(parent != nullptr && "Parent node cannot be null");
    if (!parent->east) {
        return nullptr;
    }
    auto current_leaf = parent->east;
    while (current_leaf->east) {
        current_leaf = current_leaf->east;
    }
    return current_leaf;
}


std::shared_ptr<Leaf> create_leaf(const std::shared_ptr<Node> &parent, std::string path,
                                  std::string value) {
    assert(parent != nullptr && "Parent node cannot be null");
    assert(!path.empty() && "Leaf path cannot be empty");

    auto new_leaf = std::make_shared<Leaf>();
    new_leaf->tag = Tag::Leaf;
    new_leaf->path = std::move(path);
    new_leaf->value = std::move(value);
    new_leaf->parent = parent;

    if (auto last_leaf = find_last_linear(parent)) {
        last_leaf->east = new_leaf;
        new_leaf->west = last_leaf;
    } else {
        parent->east = new_leaf;
    }

    return new_leaf;
}

// int main() {
//     auto root = create_root_node();
//     std::cout << "Корневой узел создан по адресу: " << root.get() << std::endl;
//     std::cout << "Путь корневого узла: " << root->path << std::endl;

//     auto users_node = create_node(root, "/Users");
//     std::cout << "Создан дочерний узел '" << users_node->path << "'" << std::endl;

//     // Безопасный доступ к родительскому узлу через weak_ptr
//     if (auto parent_ptr = users_node->parent.lock()) {
//         // Этот блок выполнится, только если родительский узел все еще существует
//         std::cout << "Родительский узел: " << parent_ptr->path << std::endl;
//     } else {
//         std::cout << "Родительский узел был удален." << std::endl;
//     }

//     // Демонстрация добавления листьев
//     auto bob_leaf = create_leaf(users_node, "bob", "bob_data");
//     auto kate_leaf = create_leaf(users_node, "kate", "kate_data");

//     std::cout << "\nЛистья в узле '" << users_node->path << "':" << std::endl;
//     auto current_leaf = users_node->east;
//     while (current_leaf) {
//         std::cout << "  - " << current_leaf->path << std::endl;
//         current_leaf = current_leaf->east;
//     }
//     return 0;
// }