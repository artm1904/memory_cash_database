#include "tree.hpp"

Node root = {
    .tag = (TagRoot | TagNode),
    .north = (Node*) &root,
    .west = NULL,
    .east = NULL,
    .path = "/"
};


Node* create_node (Node* parent, int8* path){

    Node* n;
    int16 size;

    assert(parent != NULL);
    assert(path != NULL);
}


int main (){
    printf("%p\n", &root);

    return 0;
}