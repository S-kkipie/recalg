#include "btree.h"

BTree::BTree() {
    root = nullptr;
}

void BTree::traverse() {
    if (root != nullptr) root->traverse();
}

BTreeNode* BTree::search(int k) {
    return (root == nullptr) ? nullptr : root->search(k);
}

void BTree::insert(int k) {
    if (root == nullptr) {
        root = new BTreeNode(true);
        root->keys[0] = k;
        root->n = 1;
    } else {
        if (root->n == 2 * T - 1) {
            BTreeNode* s = new BTreeNode(false);
            s->children[0] = root;
            s->splitChild(0, root);
            int i = 0;
            if (s->keys[0] < k)
                i++;
            s->children[i]->insertNonFull(k);
            root = s;
        } else {
            root->insertNonFull(k);
        }
    }
}

BTree::~BTree() {
    if (root) delete root;
}
