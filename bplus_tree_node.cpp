#include "bplus_tree_node.h"

template<typename KeyType, typename ValueType>
BPlusTreeNode<KeyType, ValueType>::BPlusTreeNode(bool leaf)
    : is_leaf(leaf), parent(nullptr), next_leaf(nullptr), prev_leaf(nullptr), n(0) {
    // Initializer order matches declaration order: is_leaf, (vectors), parent, next_leaf, prev_leaf, n
    // Vectors keys, values, and children are default-initialized to be empty, which is correct.
    // No specific pre-allocation is strictly needed here as they will grow dynamically.
    // For an internal node, children vector will store up to ORDER pointers.
    // For a leaf node, values vector will store up to ORDER-1 values (or ORDER in some designs,
    // but typically ORDER-1 keys means ORDER-1 values in leaves).
    // The keys vector stores up to ORDER-1 keys.

    // Let's ensure vectors are clear, though default construction does this.
    keys.clear();
    values.clear();
    children.clear();
}

template<typename KeyType, typename ValueType>
BPlusTreeNode<KeyType, ValueType>::~BPlusTreeNode() {
    // As per requirements, this destructor should not delete child nodes.
    // The BPlusTree class will manage the overall tree memory.
    // std::cout << "BPlusTreeNode destructor called." << std::endl; // Optional: for debugging
}

template<typename KeyType, typename ValueType>
int BPlusTreeNode<KeyType, ValueType>::find_key_index(KeyType key) {
    int index = 0;
    // Iterate through keys to find the first key greater than or equal to the given key
    while (index < n && keys[index] < key) {
        index++;
    }
    return index;
}

#include <cassert> // For assert

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::split_internal_node(BPlusTreeNode<KeyType, ValueType>* new_sibling_node, KeyType& key_to_parent) {
    assert(!is_leaf);
    // This method is called when the current internal node has 'ORDER' keys and 'ORDER+1' children
    // (i.e., it's overfull due to a child split).
    // new_sibling_node is empty and allocated by the caller.
    // key_to_parent will be the median key, which is moved up.

    int median_key_idx = ORDER / 2;
    key_to_parent = this->keys[median_key_idx];

    // Configure the new sibling node (right node)
    new_sibling_node->is_leaf = false;
    new_sibling_node->n = ORDER - 1 - median_key_idx; // Number of keys in the new right node
    new_sibling_node->parent = this->parent; // Parent will be set by BPlusTree class later if this is root split

    // Copy keys to the new sibling node (keys after the median key)
    new_sibling_node->keys.resize(new_sibling_node->n);
    std::copy(this->keys.begin() + median_key_idx + 1, this->keys.end(), new_sibling_node->keys.begin());

    // Copy children pointers to the new sibling node
    // Children from median_key_idx + 1 onwards
    new_sibling_node->children.resize(new_sibling_node->n + 1);
    std::copy(this->children.begin() + median_key_idx + 1, this->children.end(), new_sibling_node->children.begin());

    // Update parent pointers for the children moved to the new sibling
    for (int i = 0; i <= new_sibling_node->n; ++i) {
        if (new_sibling_node->children[i] != nullptr) {
            new_sibling_node->children[i]->parent = new_sibling_node;
        }
    }

    // Update current (left) node
    this->n = median_key_idx; // Number of keys remaining in the left node
    this->keys.resize(this->n);
    // Children up to median_key_idx (inclusive for children array)
    this->children.resize(this->n + 1);
    // Vectors are automatically truncated if new size is smaller.

    // Note: next_leaf and prev_leaf are not used for internal nodes.
}
#include <algorithm> // For std::copy, std::fill

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::split_leaf_node(BPlusTreeNode<KeyType, ValueType>* new_sibling_node, KeyType& key_to_parent) {
    assert(is_leaf);
    // This method is called when the current leaf node has 'ORDER' keys (i.e., it's overfull by 1).
    // The new_sibling_node is empty and has been allocated by the caller.
    // key_to_parent will be the first key of the new_sibling_node.

    // Number of keys for the current (left) node after split.
    int num_keys_left = ORDER / 2; // Integer division, e.g., ORDER=3 -> 1; ORDER=4 -> 2
                                   // Corrected: Standard B+ tree splits for N items (keys or pointers)
                                   // often give ceil(N/2) to left and floor(N/2) to right, or vice-versa.
                                   // Let's try left gets ceil(ORDER/2), right gets floor(ORDER/2)
                                   // when splitting ORDER keys.
    num_keys_left = (ORDER + 1) / 2; // ceil(ORDER/2.0) for integer math

    int num_keys_right = ORDER - num_keys_left;

    // Populate the new sibling node (right node)
    new_sibling_node->is_leaf = true;
    new_sibling_node->n = num_keys_right;
    new_sibling_node->parent = this->parent; // Parent will be set by BPlusTree class later if this is root split

    // Copy keys and values to the new sibling node
    new_sibling_node->keys.resize(num_keys_right);
    std::copy(this->keys.begin() + num_keys_left, this->keys.end(), new_sibling_node->keys.begin());

    new_sibling_node->values.resize(num_keys_right);
    std::copy(this->values.begin() + num_keys_left, this->values.end(), new_sibling_node->values.begin());

    // Update current (left) node
    this->n = num_keys_left;
    this->keys.resize(num_keys_left);
    this->values.resize(num_keys_left);
    // children vector is not used for leaf nodes for data pointers

    // Set the key to be copied up to the parent
    key_to_parent = new_sibling_node->keys[0];

    // Update linked list pointers for leaf nodes
    new_sibling_node->next_leaf = this->next_leaf;
    if (this->next_leaf != nullptr) {
        this->next_leaf->prev_leaf = new_sibling_node;
    }
    this->next_leaf = new_sibling_node;
    new_sibling_node->prev_leaf = this;
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::insert_non_full(KeyType key, ValueType value) {
    assert(is_leaf); // Should only be called on leaf nodes

    int idx = find_key_index(key);

    // Insert key
    if (idx < n && keys[idx] == key) {
        // Key already exists, update value (or handle as error/duplicate, depending on policy)
        // For now, let's assume updates are allowed or keys are unique and this won't happen if checked before.
        // If keys must be unique and this is an error, an exception or error code would be appropriate.
        // For simplicity in this step, we'll allow updating the value.
        values[idx] = value;
        // No change in 'n' as key already existed.
        // If duplicates were allowed, this would be an insertion.
        // std::cout << "Key " << key << " already exists, value updated." << std::endl;
        return;
    }

    // Make space for new key and value
    keys.insert(keys.begin() + idx, key);
    values.insert(values.begin() + idx, value);
    n++;
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::insert_non_full_internal(KeyType key, BPlusTreeNode<KeyType, ValueType>* new_child) {
    std::cout << "BPlusTreeNode::insert_non_full_internal not implemented yet." << std::endl;
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::traverse() {
    std::cout << "Node keys: ";
    for (int i = 0; i < n; ++i) {
        std::cout << keys[i] << " ";
    }
    std::cout << std::endl;

    if (is_leaf) {
        std::cout << "Leaf node values: ";
        for (int i = 0; i < n; ++i) {
            // Assuming ValueType is printable
            std::cout << values[i] << " ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "Internal node, traversing children:" << std::endl;
        for (int i = 0; i < n + 1; ++i) { // Internal nodes have n keys and n+1 children
            if (children[i]) {
                children[i]->traverse();
            } else {
                std::cout << "Child " << i << " is null." << std::endl;
            }
        }
    }
}

// Explicit template instantiations if needed by the build system,
// especially for separate compilation of .h and .cpp for templates.
// For now, this might not be strictly necessary, but good practice for some compilers.
// Example:
// template class BPlusTreeNode<int, int>;
// template class BPlusTreeNode<int, std::string>;
// Add other types as needed by the project.
// Without knowing specific types, it's hard to add them here.
// If the main application uses BPlusTreeNode<int, double>, that should be instantiated.
// For now, leaving this commented out or minimal.
// If compilation errors arise due to undefined template methods, these instantiations will be necessary.
// --- Implementations for Deletion Methods ---

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::remove_key_value_pair_from_leaf(int key_idx) {
    assert(is_leaf);
    assert(key_idx >= 0 && key_idx < n);

    keys.erase(keys.begin() + key_idx);
    values.erase(values.begin() + key_idx);
    n--;
}

template<typename KeyType, typename ValueType>
KeyType BPlusTreeNode<KeyType, ValueType>::get_predecessor_from_child_subtree(int child_node_idx) {
    assert(!this->is_leaf && child_node_idx >= 0 && static_cast<size_t>(child_node_idx) < this->children.size());
    BPlusTreeNode<KeyType, ValueType>* current = this->children[child_node_idx];
    // Traverse to the rightmost key in the subtree rooted at 'current'
    while (!current->is_leaf) {
        current = current->children[current->n]; // Go to the last child
    }
    // 'current' is now the leaf node containing the predecessor key
    return current->keys.back(); // The last key in this leaf
}

template<typename KeyType, typename ValueType>
KeyType BPlusTreeNode<KeyType, ValueType>::get_successor_from_child_subtree(int child_node_idx) {
    assert(!this->is_leaf && child_node_idx >= 0 && static_cast<size_t>(child_node_idx) < this->children.size());
    BPlusTreeNode<KeyType, ValueType>* current = this->children[child_node_idx];
    // Traverse to the leftmost key in the subtree rooted at 'current'
    while (!current->is_leaf) {
        current = current->children[0]; // Go to the first child
    }
    // 'current' is now the leaf node containing the successor key
    return current->keys.front(); // The first key in this leaf
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::remove_key_from_internal_node(int key_idx_to_remove, int child_ptr_idx_to_remove) {
    assert(!is_leaf);
    assert(key_idx_to_remove >= 0 && key_idx_to_remove < n);
    assert(child_ptr_idx_to_remove >= 0 && child_ptr_idx_to_remove <= n); // Can remove any child pointer

    keys.erase(keys.begin() + key_idx_to_remove);
    // The child pointer to remove depends on which key is being removed or how node is being restructured.
    // Typically, if keys[i] is removed, children[i+1] (the one to its right) is also affected or removed/merged.
    // Or, if it's just a key replacement, no child pointer is removed.
    // This helper is more for when a key AND a child pointer are removed due to a merge.
    children.erase(children.begin() + child_ptr_idx_to_remove);
    n--;
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::borrow_from_prev_leaf(BPlusTreeNode* prev_sibling, BPlusTreeNode* parent, int parent_idx_of_separator_key) {
    assert(is_leaf && prev_sibling->is_leaf);
    assert(prev_sibling->n > 0); // Must have a key to borrow

    // Take the last key/value from prev_sibling
    KeyType borrowed_key = prev_sibling->keys.back();
    ValueType borrowed_value = prev_sibling->values.back();

    // Remove from prev_sibling
    prev_sibling->keys.pop_back();
    prev_sibling->values.pop_back();
    prev_sibling->n--;

    // Add to the front of current_node (this)
    this->keys.insert(this->keys.begin(), borrowed_key);
    this->values.insert(this->values.begin(), borrowed_value);
    this->n++;

    // Update the parent's separator key
    // The key that was separating prev_sibling and this node now needs to be the new first key of this node.
    // Which is the borrowed_key.
    parent->keys[parent_idx_of_separator_key] = borrowed_key;
    // Or, more robustly, parent->keys[parent_idx_of_separator_key] = this->keys[0];
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::borrow_from_next_leaf(BPlusTreeNode* next_sibling, BPlusTreeNode* parent, int parent_idx_of_separator_key) {
    assert(is_leaf && next_sibling->is_leaf);
    assert(next_sibling->n > 0); // Must have a key to borrow

    // Take the first key/value from next_sibling
    KeyType borrowed_key = next_sibling->keys.front();
    ValueType borrowed_value = next_sibling->values.front();

    // Remove from next_sibling
    next_sibling->keys.erase(next_sibling->keys.begin());
    next_sibling->values.erase(next_sibling->values.begin());
    next_sibling->n--;

    // Add to the end of current_node (this)
    this->keys.push_back(borrowed_key);
    this->values.push_back(borrowed_value);
    this->n++;

    // Update the parent's separator key
    // The key that was separating this node and next_sibling now needs to be the new first key of next_sibling.
    parent->keys[parent_idx_of_separator_key] = next_sibling->keys[0];
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::merge_into_prev_leaf(BPlusTreeNode* prev_sibling, BPlusTreeNode* parent, int parent_idx_key_between_us) {
    assert(is_leaf && prev_sibling->is_leaf);
    assert(parent->children[parent_idx_key_between_us] == prev_sibling);
    assert(parent->children[parent_idx_key_between_us + 1] == this);

    // Move all keys and values from current_node (this) to prev_sibling
    prev_sibling->keys.insert(prev_sibling->keys.end(), this->keys.begin(), this->keys.end());
    prev_sibling->values.insert(prev_sibling->values.end(), this->values.begin(), this->values.end());
    prev_sibling->n += this->n;

    // Update linked list pointers
    prev_sibling->next_leaf = this->next_leaf;
    if (this->next_leaf != nullptr) {
        this->next_leaf->prev_leaf = prev_sibling;
    }

    // Current node (this) becomes empty and is ready for deletion by the caller.
    this->keys.clear();
    this->values.clear();
    this->n = 0;

    // The caller (BPlusTree::remove_recursive) is responsible for:
    // 1. Deleting this node.
    // 2. Removing parent->keys[parent_idx_key_between_us].
    // 3. Removing parent->children[parent_idx_key_between_us + 1] (which was the pointer to this node).
    // 4. Handling underflow in the parent if it occurs.
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::merge_next_leaf_into_current(BPlusTreeNode* next_sibling, BPlusTreeNode* parent, int parent_idx_key_between_us) {
    assert(is_leaf && next_sibling->is_leaf);
    assert(parent->children[parent_idx_key_between_us] == this);
    assert(parent->children[parent_idx_key_between_us + 1] == next_sibling);

    // Move all keys and values from next_sibling to current_node (this)
    this->keys.insert(this->keys.end(), next_sibling->keys.begin(), next_sibling->keys.end());
    this->values.insert(this->values.end(), next_sibling->values.begin(), next_sibling->values.end());
    this->n += next_sibling->n;

    // Update linked list pointers
    this->next_leaf = next_sibling->next_leaf;
    if (next_sibling->next_leaf != nullptr) {
        next_sibling->next_leaf->prev_leaf = this;
    }

    // next_sibling becomes empty and is ready for deletion by the caller.
    next_sibling->keys.clear();
    next_sibling->values.clear();
    next_sibling->n = 0;

    // The caller (BPlusTree::remove_recursive) is responsible for:
    // 1. Deleting next_sibling.
    // 2. Removing parent->keys[parent_idx_key_between_us].
    // 3. Removing parent->children[parent_idx_key_between_us + 1] (which was the pointer to next_sibling).
    // 4. Handling underflow in the parent if it occurs.
}


// --- Internal Node Borrow/Merge Implementations ---

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::borrow_from_prev_internal(BPlusTreeNode* prev_sibling, BPlusTreeNode* parent_node, int parent_key_idx) {
    assert(!this->is_leaf && !prev_sibling->is_leaf && !parent_node->is_leaf);
    assert(prev_sibling->n > 0); // Cannot borrow from an empty or minimally full node (caller should check)

    // Key from parent comes down to current node (this) as the new first key
    this->keys.insert(this->keys.begin(), parent_node->keys[parent_key_idx]);
    this->n++;

    // Parent updates its key with the last key from prev_sibling
    parent_node->keys[parent_key_idx] = prev_sibling->keys.back();
    prev_sibling->keys.pop_back();
    // prev_sibling->n--; // Will be decremented after child move

    // Last child of prev_sibling becomes first child of current_node (this)
    BPlusTreeNode<KeyType, ValueType>* child_to_move = prev_sibling->children.back();
    prev_sibling->children.pop_back();
    prev_sibling->n--; // Now decrement n for prev_sibling (one key, one child moved)

    child_to_move->parent = this;
    this->children.insert(this->children.begin(), child_to_move);
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::borrow_from_next_internal(BPlusTreeNode* next_sibling, BPlusTreeNode* parent_node, int parent_key_idx) {
    assert(!this->is_leaf && !next_sibling->is_leaf && !parent_node->is_leaf);
    assert(next_sibling->n > 0); // Cannot borrow from an empty or minimally full node

    // Key from parent comes down to current node (this) as the new last key
    this->keys.push_back(parent_node->keys[parent_key_idx]);
    this->n++;

    // Parent updates its key with the first key from next_sibling
    parent_node->keys[parent_key_idx] = next_sibling->keys.front();
    next_sibling->keys.erase(next_sibling->keys.begin());
    // next_sibling->n--; // Will be decremented after child move

    // First child of next_sibling becomes last child of current_node (this)
    BPlusTreeNode<KeyType, ValueType>* child_to_move = next_sibling->children.front();
    next_sibling->children.erase(next_sibling->children.begin());
    next_sibling->n--; // Now decrement n for next_sibling

    child_to_move->parent = this;
    this->children.push_back(child_to_move);
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::merge_into_prev_internal(BPlusTreeNode* prev_sibling, BPlusTreeNode* parent_node, int parent_key_idx) {
    assert(!this->is_leaf && !prev_sibling->is_leaf && !parent_node->is_leaf);
    assert(parent_node->children[parent_key_idx] == prev_sibling);
    assert(parent_node->children[parent_key_idx + 1] == this);

    // Key from parent comes down into prev_sibling
    prev_sibling->keys.push_back(parent_node->keys[parent_key_idx]);
    prev_sibling->n++;

    // Move all keys and children from current_node (this) to prev_sibling
    prev_sibling->keys.insert(prev_sibling->keys.end(), this->keys.begin(), this->keys.end());
    prev_sibling->children.insert(prev_sibling->children.end(), this->children.begin(), this->children.end());

    // Update parent pointers for the moved children
    for (size_t i = 0; i < this->children.size(); ++i) {
        this->children[i]->parent = prev_sibling;
    }
    prev_sibling->n += this->n;

    // Current node (this) becomes empty and is ready for deletion by the caller.
    this->keys.clear();
    this->children.clear(); // Important to avoid double deletion of children by this node's destructor
    this->n = 0;

    // Parent node loses a key and a child pointer (the one to 'this' node)
    // This is handled by the caller (BPlusTree::remove_recursive) after this method returns,
    // typically by calling parent_node->remove_key_from_internal_node(parent_key_idx, parent_key_idx + 1);
    // and then deleting 'this' node.
}

template<typename KeyType, typename ValueType>
void BPlusTreeNode<KeyType, ValueType>::merge_next_internal_into_current(BPlusTreeNode* next_sibling, BPlusTreeNode* parent_node, int parent_key_idx) {
    assert(!this->is_leaf && !next_sibling->is_leaf && !parent_node->is_leaf);
    assert(parent_node->children[parent_key_idx] == this);
    assert(parent_node->children[parent_key_idx + 1] == next_sibling);

    // Key from parent comes down into current_node (this)
    this->keys.push_back(parent_node->keys[parent_key_idx]);
    this->n++;

    // Move all keys and children from next_sibling to current_node (this)
    this->keys.insert(this->keys.end(), next_sibling->keys.begin(), next_sibling->keys.end());
    this->children.insert(this->children.end(), next_sibling->children.begin(), next_sibling->children.end());

    // Update parent pointers for the moved children
    for (size_t i = 0; i < next_sibling->children.size(); ++i) {
        next_sibling->children[i]->parent = this;
    }
    this->n += next_sibling->n;

    // next_sibling becomes empty and is ready for deletion by the caller.
    next_sibling->keys.clear();
    next_sibling->children.clear(); // Important to avoid double deletion
    next_sibling->n = 0;

    // Parent node loses a key and a child pointer (the one to 'next_sibling' node)
    // This is handled by the caller (BPlusTree::remove_recursive) after this method returns,
    // typically by calling parent_node->remove_key_from_internal_node(parent_key_idx, parent_key_idx + 1);
    // and then deleting 'next_sibling' node.
}


template class BPlusTreeNode<int, int>;
template class BPlusTreeNode<int, double>; // Assuming int keys and double values as a common case
template class BPlusTreeNode<int, std::string>; // Assuming int keys and string values
template class BPlusTreeNode<std::string, std::string>; // For string tests
template class BPlusTreeNode<std::string, int>; // For string key, int value tests
// Add more instantiations based on actual usage in the project.
