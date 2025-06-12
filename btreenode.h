#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include <vector>
#include <iostream>

const int T = 3;

class BTreeNode {
public:
    std::vector<int> keys;
    std::vector<BTreeNode*> children;
    int n;
    bool leaf;

    BTreeNode(bool _leaf);
    void traverse();
    BTreeNode* search(int k);
    void insertNonFull(int k);
    void splitChild(int i, BTreeNode* y);
    ~BTreeNode();
};

#endif // BTREE_NODE_H
