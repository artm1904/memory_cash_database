#include "tree.hpp"

void print_tree_helper(const std::shared_ptr<Node> &node, int indent) {
    if (!node) {
        return;
    }

    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";  // Отступ для отображения иерархии
    }
    std::cout << "📁 "<< node->path << std::endl;

    // Рекурсивно вызываем для дочерних узлов и листьев, увеличивая отступ

    for (auto child : node->childs) {
        print_tree_helper(child, indent + 1);
    }

    auto current_leaf = node->east;
    while (current_leaf) {
        for (int i = 0; i < indent + 1; ++i) {
            std::cout << "  ";
        }
        std::cout << "🍃 " << current_leaf->path << " (value: '" << current_leaf->value << "')"
                  << std::endl;
        current_leaf = current_leaf->east;
    }
}

// Рекурсивная вспомогательная функция для формирования строкового представления дерева.
static void print_tree_recursive(std::stringstream &ss, const std::shared_ptr<Node> &node,
                                 int indent) {
    if (!node) {
        return;
    }

    // Выводим текущий узел с отступом
    ss << std::string(indent * 2, ' ') << "📁 " << node->path << "\n";

    // Рекурсивно вызываем для дочерних узлов
    for (const auto &child : node->childs) {
        print_tree_recursive(ss, child, indent + 1);
    }

    // Итерируемся по листьям текущего узла
    auto current_leaf = node->east;
    while (current_leaf) {
        ss << std::string((indent + 1) * 2, ' ') << "🍃 " << current_leaf->path << " (value: '"
           << current_leaf->value << "')\n";
        current_leaf = current_leaf->east;
    }
}

// Рекурсивная вспомогательная функция для поиска узла по полному пути.
static std::shared_ptr<Node> find_node_recursive(const std::shared_ptr<Node> &current_node,
                                                 const std::string &path) {
    if (!current_node) {
        return nullptr;
    }

    // 1. Проверяем, является ли текущий узел искомым.
    if (current_node->path == path) {
        return current_node;
    }

    // 2. Если нет, рекурсивно ищем во всех дочерних узлах.
    for (const auto &child : current_node->childs) {
        auto found = find_node_recursive(child, path);
        if (found) {
            // Если нашли в дочернем поддереве, возвращаем результат.
            return found;
        }
    }

    // 3. Если не нашли ни в текущем узле, ни в его поддеревьях.
    return nullptr;
}

/*------------------------------------HEADER_FUNCTION_IMPLEMENTATION--------------------------------------------*/

std::shared_ptr<Node> create_root_node() {
    auto root = std::make_shared<Node>();
    root->tag = Tag::Root | Tag::Node;
    root->path = "/";
    // Дочерние указатели по умолчанию равны nullptr в std::shared_ptr
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

void print_tree(const std::shared_ptr<Node> &root) { print_tree_helper(root, 0); }

std::string print_tree_string(const std::shared_ptr<Node> &root) {
    if (!root) {
        return "";
    }
    std::stringstream ss;
    print_tree_recursive(ss, root, 0);
    return ss.str();
}

std::shared_ptr<Node> find_node_by_path_linear(const std::shared_ptr<Node> &root,
                                               const std::string &path) {
    if (!root || path.empty()) {
        return nullptr;
    }
    return find_node_recursive(root, path);
}

bool delete_node_by_path_linear(const std::shared_ptr<Node> &root, const std::string &path) {
    if (path == "/") {
        std::cerr << "Error: Cannot delete the root node." << std::endl;
        return false;
    }

    auto node_to_delete = find_node_by_path_linear(root, path);
    if (!node_to_delete) {
        std::cerr << "Error: Node '" << path << "' not found for deletion." << std::endl;
        return false;
    }

    // 2. Получаем родителя этого узла.
    auto parent_node = node_to_delete->parent.lock();

    // 3. Удаляем узел из списка дочерних элементов родителя.
    // Используем идиому erase-remove для эффективного удаления.
    auto &children = parent_node->childs;
    auto it = std::remove(children.begin(), children.end(), node_to_delete);

    if (it == children.end()) {
        std::cerr << "Consistency Error: Node found but not in parent's child list." << std::endl;
        return false;
    }

    children.erase(it, children.end());
    return true;
}

std::shared_ptr<Leaf> find_leaf_by_path_linear(const std::shared_ptr<Node> &root,
                                               const std::string &path) {
    if (!root || path.empty() || path == "/" || path.find('/') == std::string::npos) {
        // Путь не может быть пустым, корневым или не содержать '/'
        return nullptr;
    }

    // 1. Определяем путь к родительскому узлу
    size_t last_slash_pos = path.rfind('/');
    std::string parent_path = (last_slash_pos == 0) ? "/" : path.substr(0, last_slash_pos);

    // 2. Находим родительский узел
    auto parent_node = find_node_by_path_linear(root, parent_path);
    if (!parent_node) {
        // Родитель не найден, значит и листа быть не может
        return nullptr;
    }

    // 3. Ищем лист в связанном списке родителя
    auto current_leaf = parent_node->east;
    while (current_leaf) {
        if (current_leaf->path == path) {
            return current_leaf;  // Нашли!
        }
        current_leaf = current_leaf->east;
    }

    // Лист не найден в списке родителя
    return nullptr;
}

bool delete_leaf_by_path_linear(const std::shared_ptr<Node> &root, const std::string &path) {
    // 1. Находим лист, который нужно удалить.
    auto leaf_to_delete = find_leaf_by_path_linear(root, path);
    if (!leaf_to_delete) {
        std::cerr << "Error: Leaf '" << path << "' not found for deletion." << std::endl;
        return false;
    }

    // 2. Получаем указатели на соседние листья.
    auto prev_leaf = leaf_to_delete->west;
    auto next_leaf = leaf_to_delete->east;

    // 3. Обновляем связи в двусвязном списке.
    if (prev_leaf) {
        // Если есть предыдущий лист, его 'east' теперь указывает на следующий.
        prev_leaf->east = next_leaf;
    } else {
        // Если предыдущего листа нет, значит, удаляемый лист был первым.
        // Нужно обновить указатель 'east' у родительского узла.
        if (std::holds_alternative<std::weak_ptr<Node>>(leaf_to_delete->parent)) {
            auto parent_node = std::get<std::weak_ptr<Node>>(leaf_to_delete->parent).lock();
            if (parent_node) {
                parent_node->east = next_leaf;
            } else {
                std::cerr << "Consistency Error: Could not lock parent node of a leaf."
                          << std::endl;
                return false;
            }
        }
    }

    if (next_leaf) {
        // Если есть следующий лист, его 'west' теперь указывает на предыдущий.
        next_leaf->west = prev_leaf;
    }

    return true;
}

std::shared_ptr<Node> create_node_by_path(const std::shared_ptr<Node> &root,
                                          const std::string &path) {
    if (path.empty() || path == "/" || path.find('/') == std::string::npos || path.back() == '/') {
        std::cerr << "Error: Invalid path for new node: '" << path << "'" << std::endl;
        return nullptr;
    }

    // 1. Проверяем, не существует ли уже узел или лист с таким путем
    if (find_node_by_path_linear(root, path) || find_leaf_by_path_linear(root, path)) {
        std::cerr << "Error: Node or leaf with path '" << path << "' already exists." << std::endl;
        return nullptr;
    }

    // 2. Определяем путь к родительскому узлу
    size_t last_slash_pos = path.rfind('/');
    std::string parent_path = (last_slash_pos == 0) ? "/" : path.substr(0, last_slash_pos);

    // 3. Находим родительский узел
    auto parent_node = find_node_by_path_linear(root, parent_path);
    if (!parent_node) {
        std::cerr << "Error: Parent node '" << parent_path << "' not found. Cannot create node '"
                  << path << "'." << std::endl;
        return nullptr;
    }

    // 4. Если все проверки пройдены, создаем новый узел
    return create_node(parent_node, path);
}

std::shared_ptr<Leaf> create_leaf_by_path(const std::shared_ptr<Node> &root,
                                          const std::string &path, const std::string &value) {
    if (path.empty() || path == "/" || path.find('/') == std::string::npos || path.back() == '/') {
        std::cerr << "Error: Invalid path for new leaf: '" << path << "'" << std::endl;
        return nullptr;
    }

    // 1. Проверяем, не существует ли уже узел или лист с таким путем
    if (find_node_by_path_linear(root, path) || find_leaf_by_path_linear(root, path)) {
        std::cerr << "Error: Node or leaf with path '" << path << "' already exists." << std::endl;
        return nullptr;
    }

    // 2. Определяем путь к родительскому узлу
    size_t last_slash_pos = path.rfind('/');
    std::string parent_path = (last_slash_pos == 0) ? "/" : path.substr(0, last_slash_pos);

    // 3. Находим родительский узел
    auto parent_node = find_node_by_path_linear(root, parent_path);
    if (!parent_node) {
        std::cerr << "Error: Parent node '" << parent_path << "' not found. Cannot create leaf '"
                  << path << "'." << std::endl;
        return nullptr;
    }

    // 4. Если все проверки пройдены, создаем новый лист
    return create_leaf(parent_node, path, value);
}

// int main() {
//     auto root = create_root_node();
//     // В этой реализации поле path хранит полный путь до узла/листа.
//     auto users_node = create_node(root, "/Users");
//     auto shops_node = create_node(root, "/Shops");

//     auto login_node = create_node(users_node, "/Users/Login");
//     auto password_node = create_node(users_node, "/Users/Password");

//     auto bob_leaf = create_leaf(login_node, "/Users/Login/bob", "bob_data");
//     auto kate_leaf = create_leaf(login_node, "/Users/Login/kate", "kate_data");

//     std::cout << "--- Initial Tree ---" << std::endl;
//     //std::cout << print_tree_string(root);
//     print_tree(root);
//     std::cout << "--------------------" << std::endl;

//     std::cout << "\nSearching for /Users/Login..." << std::endl;
//     auto found_node = find_node_by_path_linear(root, "/Users/Login");
//     if (found_node) {
//         std::cout << "Found node with path: " << found_node->path << std::endl;
//     } else {
//         std::cout << "Node /Users/Login not found." << std::endl;
//     }

//     std::cout << "\n--- Deleting node /Users/Password ---" << std::endl;
//     if (delete_node_by_path_linear(root, "/Users/Password")) {
//         std::cout << "Node /Users/Password deleted successfully." << std::endl;
//         std::cout << "\n--- Tree after deletion ---" << std::endl;
//         print_tree(root);
//         std::cout << "---------------------------" << std::endl;
//     } else {
//         std::cout << "Failed to delete node /Users/Password." << std::endl;
//     }

//     std::cout << "\n--- Searching for leaf /Users/Login/bob ---" << std::endl;
//     auto found_leaf = find_leaf_by_path_linear(root, "/Users/Login/bob");
//     if (found_leaf) {
//         std::cout << "Found leaf with path: " << found_leaf->path << ", and value: '"
//                   << found_leaf->value << "'" << std::endl;
//     } else {
//         std::cout << "Leaf /Users/Login/bob not found." << std::endl;
//     }

//     std::cout << "\n--- Searching for non-existent leaf ---" << std::endl;
//     if (!find_leaf_by_path_linear(root, "/Users/Login/non_existent")) {
//         std::cout << "Leaf /Users/Login/non_existent correctly not found." << std::endl;
//     }

//     std::cout << "\n--- Creating new node /Users/Profile ---" << std::endl;
//     if (auto new_node = create_node_by_path(root, "/Users/Profile")) {
//         std::cout << "Node " << new_node->path << " created successfully." << std::endl;
//     } else {
//         std::cout << "Failed to create node /Users/Profile." << std::endl;
//     }

//     std::cout << "\n--- Creating new leaf /Users/Profile/settings ---" << std::endl;
//     if (auto new_leaf = create_leaf_by_path(root, "/Users/Profile/settings", "dark_theme")) {
//         std::cout << "Leaf " << new_leaf->path << " with value '" << new_leaf->value
//                   << "' created successfully." << std::endl;
//     }

//     std::cout << "\n--- Final tree state ---" << std::endl;
//     print_tree(root);

//     return 0;
// }
