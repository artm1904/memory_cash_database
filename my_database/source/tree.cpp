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

    parent->childs.push_back(new_node);
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
void print_tree(const std::shared_ptr<Leaf> &leaf) {
    if (!leaf) {
        return;
    }

    std::cout << leaf->path << std::endl;

    if (leaf->east) {
        print_tree(leaf->east);
    }
}

void print_tree_helper(const std::shared_ptr<Node> &node, int indent) {
    if (!node) {
        return;
    }

    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";  // Отступ для отображения иерархии
    }
    std::cout << node->path << std::endl;

    // Рекурсивно вызываем для дочерних узлов и листьев, увеличивая отступ
    
    for(auto child : node->childs){
        print_tree_helper(child, indent + 1);
    
    }
    if (node->east) {
        // Для листьев отступ не увеличиваем, так как они находятся на том же уровне
        print_tree(node->east);
    }
}

void print_tree(const std::shared_ptr<Node> &root) { print_tree_helper(root, 0); }

int main() {
    auto root = create_root_node();
    auto users_node = create_node(root, "/Users");
    auto shops_node = create_node(root, "/Shops");

    auto login_node = create_node(users_node, "/Users/Login");
    auto password_node = create_node(users_node, "/Users/Password");

    auto bob_leaf = create_leaf(login_node, "/Users/Login/bob", "bob_data");
    auto kate_leaf = create_leaf(login_node, "/Users/Login/kate", "kate_data");

    print_tree(root);
    return 0;
}