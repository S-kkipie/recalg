#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include "bplus_tree_node.h" // Includes <vector>, <iostream> via this
#include <optional>           // For std::optional in search method

// Forward declaration of BPlusTreeNode if not fully included by bplus_tree_node.h for some reason
// template<typename KeyType, typename ValueType> class BPlusTreeNode;
// No, bplus_tree_node.h should provide the full definition.

template<typename KeyType, typename ValueType>
class BPlusTree {
public:
    // Constructor
    BPlusTree();

    // Destructor
    ~BPlusTree();

    // Public Methods
    void insert(KeyType key, ValueType value);
    std::optional<ValueType> search(KeyType key);
    void remove(KeyType key); // Declaration only for now
    void traverse();          // For debugging
    BPlusTreeNode<KeyType, ValueType>* get_first_leaf(); // For testing and range scans

private:
    BPlusTreeNode<KeyType, ValueType>* root;
    // int order; // Using const ORDER from bplus_tree_node.h for now

    // Private Helper Methods
    void destroy_tree(BPlusTreeNode<KeyType, ValueType>* node);
    BPlusTreeNode<KeyType, ValueType>* find_leaf_node(KeyType key); // For search and insert/delete path

    void insert_recursive(
        BPlusTreeNode<KeyType, ValueType>* current_node,
        KeyType key,
        ValueType value,
        KeyType& key_promoted_up, // Output: key to be promoted if current_node splits
        BPlusTreeNode<KeyType, ValueType>*& new_sibling_of_current // Output: new node created if current_node splits
    );

    // child_idx_in_parent is the index of current_node in parent_node->children[]
    void remove_recursive(
        BPlusTreeNode<KeyType, ValueType>* current_node,
        KeyType key_to_delete,
        BPlusTreeNode<KeyType, ValueType>* parent_node,
        int child_idx_in_parent
    );


};

#endif // BPLUS_TREE_H
