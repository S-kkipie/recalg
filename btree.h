#ifndef BTREE_H
#define BTREE_H

#include "btreenode.h"

class BTree {
public:
    BTreeNode* root;
    BTree();
    void traverse();
    BTreeNode* search(int k);
    void insert(int k);
    ~BTree();
};

#endif // BTREE_H
