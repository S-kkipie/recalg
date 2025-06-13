#ifndef BPLUS_TREE_NODE_H
#define BPLUS_TREE_NODE_H

#include <vector>
#include <iostream>

// Order of the B+ Tree. Determines the maximum number of keys/children.
const int ORDER = 3;

template<typename KeyType, typename ValueType>
class BPlusTreeNode {
public:
    // Member Variables
    bool is_leaf;
    std::vector<KeyType> keys;
    std::vector<BPlusTreeNode<KeyType, ValueType>*> children; // For internal nodes
    std::vector<ValueType> values; // For leaf nodes
    BPlusTreeNode<KeyType, ValueType>* parent;
    BPlusTreeNode<KeyType, ValueType>* next_leaf; // For leaf nodes
    BPlusTreeNode<KeyType, ValueType>* prev_leaf; // For leaf nodes
    int n; // Current number of keys

    // Constructor
    BPlusTreeNode(bool leaf);

    // Destructor
    ~BPlusTreeNode();

    // Method Declarations
    int find_key_index(KeyType key);
    void split_internal_node(BPlusTreeNode<KeyType, ValueType>* new_sibling_node, KeyType& key_to_parent); // Renamed
    void split_leaf_node(BPlusTreeNode<KeyType, ValueType>* new_sibling_node, KeyType& key_to_parent);     // Renamed
    void insert_non_full(KeyType key, ValueType value); // For leaf nodes
    void insert_non_full_internal(KeyType key, BPlusTreeNode<KeyType, ValueType>* new_child); // For internal nodes
    void traverse(); // For debugging

    // --- Methods for Deletion ---
    // Helper to remove a key (and value if leaf, or child if internal) from a node
    void remove_key_value_pair_from_leaf(int key_idx); // For leaf
    // void remove_internal_key_child_pair(int key_idx, int child_idx_to_remove); // For internal (placeholder for now)

    // Method to find predecessor/successor key (for internal node key replacement)
    // These are called on an internal node 'this'.
    // child_node_idx is the index in this->children[] that is the root of the subtree to search.
    KeyType get_predecessor_from_child_subtree(int child_node_idx);
    KeyType get_successor_from_child_subtree(int child_node_idx);

    void remove_key_from_internal_node(int key_idx_to_remove, int child_ptr_idx_to_remove);


    // Methods for handling underflow after deletion from a leaf
    // Parameters: sibling node, parent node, index of key in parent separating this node from its sibling
    // or index of this node in parent's children if that's more convenient for parent key update.
    void borrow_from_prev_leaf(BPlusTreeNode* prev_sibling, BPlusTreeNode* parent, int parent_idx_of_separator_key);
    void borrow_from_next_leaf(BPlusTreeNode* next_sibling, BPlusTreeNode* parent, int parent_idx_of_separator_key);
    // merge_with_prev_leaf: 'this' node's content goes into prev_sibling. 'this' node is then deleted by caller.
    // parent_idx_key_between is the index of the key in parent that is between prev_sibling and 'this'.
    void merge_into_prev_leaf(BPlusTreeNode* prev_sibling, BPlusTreeNode* parent, int parent_idx_key_between_us);
    // merge_with_next_leaf: next_sibling's content goes into 'this' node. next_sibling is then deleted by caller.
    // parent_idx_key_between is the index of the key in parent that is between 'this' and next_sibling.
    void merge_next_leaf_into_current(BPlusTreeNode* next_sibling, BPlusTreeNode* parent, int parent_idx_key_between_us);

    // Methods for handling underflow after deletion from an internal node (Placeholders for now)
    // Methods for handling underflow after deletion from an internal node
    // Called on the child node that is underflowing. Sibling is its direct sibling.
    // parent_node is their direct parent. parent_key_idx is the index of the key in parent_node that separates them.
    void borrow_from_prev_internal(BPlusTreeNode* prev_sibling, BPlusTreeNode* parent_node, int parent_key_idx);
    void borrow_from_next_internal(BPlusTreeNode* next_sibling, BPlusTreeNode* parent_node, int parent_key_idx);
    void merge_into_prev_internal(BPlusTreeNode* prev_sibling, BPlusTreeNode* parent_node, int parent_key_idx);
    void merge_next_internal_into_current(BPlusTreeNode* next_sibling, BPlusTreeNode* parent_node, int parent_key_idx);

};

#endif // BPLUS_TREE_NODE_H
