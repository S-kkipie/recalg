#include "btreenode.h"

BTreeNode::BTreeNode(bool _leaf) {
    leaf = _leaf;
    n = 0;
    keys.resize(2 * T - 1);
    children.resize(2 * T);
}

void BTreeNode::traverse() {
    int i;
    for (i = 0; i < n; i++) {
        if (!leaf)
            children[i]->traverse();
        std::cout << " " << keys[i];
    }
    if (!leaf)
        children[i]->traverse();
}

BTreeNode* BTreeNode::search(int k) {
    int i = 0;
    while (i < n && k > keys[i])
        i++;
    if (i < n && keys[i] == k)
        return this;
    if (leaf)
        return nullptr;
    return children[i]->search(k);
}

void BTreeNode::insertNonFull(int k) {
    int i = n - 1;
    if (leaf) {
        while (i >= 0 && keys[i] > k) {
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = k;
        n = n + 1;
    } else {
        while (i >= 0 && keys[i] > k)
            i--;
        if (children[i + 1]->n == 2 * T - 1) {
            splitChild(i + 1, children[i + 1]);
            if (keys[i + 1] < k)
                i++;
        }
        children[i + 1]->insertNonFull(k);
    }
}

void BTreeNode::splitChild(int i, BTreeNode* y) {
    BTreeNode* z = new BTreeNode(y->leaf);
    z->n = T - 1;
    for (int j = 0; j < T - 1; j++)
        z->keys[j] = y->keys[j + T];
    if (!y->leaf) {
        for (int j = 0; j < T; j++)
            z->children[j] = y->children[j + T];
    }
    y->n = T - 1;
    for (int j = n; j >= i + 1; j--)
        children[j + 1] = children[j];
    children[i + 1] = z;
    for (int j = n - 1; j >= i; j--)
        keys[j + 1] = keys[j];
    keys[i] = y->keys[T - 1];
    n = n + 1;
}

BTreeNode::~BTreeNode() {
    for (int i = 0; i <= n; i++) {
        if (!leaf && children[i])
            delete children[i];
    }
}
