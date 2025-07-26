#include <cassert>

#include <stdio.h>
#include <variant>


#define TagRoot 1 /* 00 01*/
#define TagNode 2 /* 00 10*/
#define TagLeaf 4 /* 01 00*/



using int32 = unsigned int;
using int16 = unsigned short;
using int8 = unsigned char;
using Tag = unsigned char;


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

struct s_node {
    Tag tag;
  struct s_node* north;
  struct s_node* west;
  struct s_leaf* east;
  int8 path[256];
};

using Node = struct s_node;

struct s_leaf {
    Tag tag;
  std::variant<s_node*, s_leaf*> west;
  struct s_leaf* north;
  struct s_leaf* east;
  int8 path[256];
  int8* value;  // We could have a big JSON stracture, which requires a lot of memory (allocate in
                // heap)
  int16 size;   // size of the value information;
};

using Leaf = struct s_leaf;

