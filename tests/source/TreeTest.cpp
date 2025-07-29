#include <gtest/gtest.h>

#include "tree.hpp"

namespace database_test {

// Тестовый класс (fixture) для создания общего окружения для тестов дерева.
// Это позволяет нам не создавать корневой узел в каждом тесте заново.
class TreeTest : public ::testing::Test {
protected:
    // Этот метод будет вызываться перед каждым тестом.
    void SetUp() override { root = create_root_node(); }

    // Объекты, объявленные здесь, доступны во всех тестах этого класса.
    std::shared_ptr<Node> root;
};

TEST_F(TreeTest, RootNodeCreation) {
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->path, "/");
    // Проверяем, что установлены флаги и Root, и Node
    EXPECT_EQ(static_cast<unsigned char>(root->tag),
              static_cast<unsigned char>(Tag::Root | Tag::Node));
    // У корневого узла не должно быть родителя
    EXPECT_TRUE(root->parent.expired());
}

TEST_F(TreeTest, ChildNodeCreation) {
    auto users_node = create_node(root, "Users");

    ASSERT_NE(users_node, nullptr);
    EXPECT_EQ(users_node->path, "Users");
    EXPECT_EQ(static_cast<unsigned char>(users_node->tag), static_cast<unsigned char>(Tag::Node));

    // Проверяем связь родитель-потомок
    ASSERT_FALSE(users_node->parent.expired());
    EXPECT_EQ(users_node->parent.lock(), root);
    ASSERT_EQ(root->childs.size(), 1);
    EXPECT_EQ(root->childs[0], users_node);
}

TEST_F(TreeTest, SingleLeafCreation) {
    auto users_node = create_node(root, "Users");
    auto bob_leaf = create_leaf(users_node, "bob", "bob_data");

    ASSERT_NE(bob_leaf, nullptr);
    EXPECT_EQ(bob_leaf->path, "bob");
    EXPECT_EQ(bob_leaf->value, "bob_data");
    EXPECT_EQ(static_cast<unsigned char>(bob_leaf->tag), static_cast<unsigned char>(Tag::Leaf));

    // Проверяем, что родитель листа - это узел `users_node`
    ASSERT_TRUE(std::holds_alternative<std::weak_ptr<Node>>(bob_leaf->parent));
    auto parent_node = std::get<std::weak_ptr<Node>>(bob_leaf->parent).lock();
    ASSERT_NE(parent_node, nullptr);
    EXPECT_EQ(parent_node, users_node);

    // Проверяем, что это первый и единственный лист в списке
    EXPECT_EQ(users_node->east, bob_leaf);
    EXPECT_EQ(bob_leaf->west, nullptr);
    EXPECT_EQ(bob_leaf->east, nullptr);
}

TEST_F(TreeTest, MultipleLeafCreationAndTraversal) {
    auto users_node = create_node(root, "Users");
    auto bob_leaf = create_leaf(users_node, "bob", "bob_data");
    auto kate_leaf = create_leaf(users_node, "kate", "kate_data");

    // Проверяем целостность двусвязного списка листьев
    ASSERT_EQ(users_node->east, bob_leaf);
    ASSERT_EQ(bob_leaf->east, kate_leaf);
    ASSERT_EQ(kate_leaf->west, bob_leaf);
    EXPECT_EQ(kate_leaf->east, nullptr);  // kate - последний элемент
    EXPECT_EQ(bob_leaf->west, nullptr);   // bob - первый элемент
}


TEST_F(TreeTest, PrintTree) {
    auto users_node = create_node(root, "/Users");
    auto bob_leaf = create_leaf(users_node, "/Users/bob", "bob_data");
    auto kate_leaf = create_leaf(users_node, "/Users/kate", "kate_data");

    print_tree(root);
}

}  // namespace database_test
