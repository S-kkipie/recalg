#include "bplus_tree.h"
#include <iostream> // For std::cout in traverse and stubs

// Constructor
template<typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::BPlusTree() : root(nullptr) {
    // std::cout << "BPlusTree initialized." << std::endl;
}

// Destructor
template<typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::~BPlusTree() {
    destroy_tree(root);
    // std::cout << "BPlusTree destroyed." << std::endl;
}

// Private Helper: destroy_tree
template<typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::destroy_tree(BPlusTreeNode<KeyType, ValueType>* node) {
    if (node != nullptr) {
        if (!node->is_leaf) {
            for (int i = 0; i < node->n + 1; ++i) { // Internal nodes have n keys and n+1 children
                if (node->children[i] != nullptr) { // Check if child pointer is valid
                    destroy_tree(node->children[i]);
                }
            }
        }
        // For leaf nodes, there are no children to deallocate via the 'children' vector.
        // The 'values' vector in leaf nodes holds ValueType, not pointers to be deleted here,
        // unless ValueType itself is a pointer type, which needs careful handling by the user.
        delete node;
    }
}

// Public Method: traverse
template<typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::traverse() {
    if (root == nullptr) {
        std::cout << "Tree is empty." << std::endl;
    } else {
        root->traverse(); // Assumes BPlusTreeNode::traverse() is implemented
    }
}

#include <algorithm> // Required for std::sort, std::lower_bound, std::copy, etc. in recursive helper

// Public Method: insert
template<typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::insert(KeyType key, ValueType value) {
    if (root == nullptr) {
        root = new BPlusTreeNode<KeyType, ValueType>(true); // Create a leaf root
        root->keys.push_back(key);
        root->values.push_back(value);
        root->n = 1;
        return;
    }

    KeyType key_promoted_up; // Will hold the key promoted if the current root splits
    BPlusTreeNode<KeyType, ValueType>* new_sibling_of_root = nullptr; // Will point to the new sibling if root splits

    insert_recursive(root, key, value, key_promoted_up, new_sibling_of_root);

    if (new_sibling_of_root != nullptr) {
        // Root was split, create a new root
        BPlusTreeNode<KeyType, ValueType>* new_root = new BPlusTreeNode<KeyType, ValueType>(false); // New root is internal
        new_root->keys.push_back(key_promoted_up);
        new_root->children.push_back(root); // Old root is the first child
        new_root->children.push_back(new_sibling_of_root);
        new_root->n = 1; // New root has one key and two children

        root->parent = new_root;
        new_sibling_of_root->parent = new_root;

        root = new_root; // Update tree's root pointer
    }
}


// Public method: get_first_leaf (for testing and range scans)
template<typename KeyType, typename ValueType>
BPlusTreeNode<KeyType, ValueType>* BPlusTree<KeyType, ValueType>::get_first_leaf() {
    if (!root) {
        return nullptr;
    }
    BPlusTreeNode<KeyType, ValueType>* current = root;
    while (current && !current->is_leaf) {
        if (current->children.empty()) {
            // This case should ideally not be reached in a valid B+Tree if current is not a leaf.
            // However, if it does, it means an internal node has no children, which is an invalid state.
            return nullptr;
        }
        current = current->children[0];
    }
    return current; // current is either a leaf or nullptr if tree structure is invalid/empty path found
}


template<typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::insert_recursive(
    BPlusTreeNode<KeyType, ValueType>* current_node,
    KeyType key,
    ValueType value,
    KeyType& key_promoted_up, // Output: key to be promoted if current_node splits
    BPlusTreeNode<KeyType, ValueType>*& new_sibling_of_current // Output: new node created if current_node splits
) {
    new_sibling_of_current = nullptr; // Default: no split at this level initially

    if (current_node->is_leaf) {
        if (current_node->n < ORDER - 1) { // Leaf has space
            current_node->insert_non_full(key, value);
            // No key promoted, no new sibling created at this level
            // key_promoted_up can be left as is, or cleared: key_promoted_up = KeyType();
        } else { // Leaf is full, must split
            // Temporarily insert the new key/value to make the node overfull (n == ORDER)
            // This prepares it for the split_leaf_node logic which expects an overfull node.
            int idx = current_node->find_key_index(key);
            current_node->keys.insert(current_node->keys.begin() + idx, key);
            current_node->values.insert(current_node->values.begin() + idx, value);
            current_node->n++;

            // Perform the split
            new_sibling_of_current = new BPlusTreeNode<KeyType, ValueType>(true); // New leaf node
            new_sibling_of_current->parent = current_node->parent; // Will be updated by caller if parent splits or if this is root

            current_node->split_leaf_node(new_sibling_of_current, key_promoted_up);
            // split_leaf_node handles setting n for both current_node and new_sibling_of_current,
            // and also updates next_leaf/prev_leaf pointers.
        }
    } else { // Internal node
        // Find the child to descend into
        int child_idx = current_node->find_key_index(key);
        if (child_idx < current_node->n && current_node->keys[child_idx] == key) {
            // Key matches an entry in internal node, descend to the right child of that key
            child_idx++;
        }
        // else, key < keys[child_idx] or key >= all keys, descend children[child_idx]

        BPlusTreeNode<KeyType, ValueType>* child_node_to_descend = current_node->children[child_idx];

        KeyType key_promoted_from_child;
        BPlusTreeNode<KeyType, ValueType>* new_sibling_from_child_split = nullptr;

        insert_recursive(child_node_to_descend, key, value, key_promoted_from_child, new_sibling_from_child_split);

        if (new_sibling_from_child_split != nullptr) { // Child was split
            // Insert the promoted key and the new sibling pointer into current_node
            int insertion_idx = current_node->find_key_index(key_promoted_from_child);
            current_node->keys.insert(current_node->keys.begin() + insertion_idx, key_promoted_from_child);
            current_node->children.insert(current_node->children.begin() + insertion_idx + 1, new_sibling_from_child_split);
            current_node->n++;

            // Important: Set parent of the new child that was just inserted
            new_sibling_from_child_split->parent = current_node;

            if (current_node->n == ORDER) { // Current internal node is now overfull, must split
                new_sibling_of_current = new BPlusTreeNode<KeyType, ValueType>(false); // New internal node
                new_sibling_of_current->parent = current_node->parent; // Will be updated by caller

                current_node->split_internal_node(new_sibling_of_current, key_promoted_up);
                // split_internal_node updates parent pointers of children moved to new_sibling_of_current.
            }
            // If no split of current_node, new_sibling_of_current remains nullptr (already set)
            // and key_promoted_up is not set by this level.
        }
        // If child was not split, new_sibling_of_current remains nullptr, key_promoted_up is not set.
    }
}


// Public Method: search
template<typename KeyType, typename ValueType>
std::optional<ValueType> BPlusTree<KeyType, ValueType>::search(KeyType key) {
    BPlusTreeNode<KeyType, ValueType>* leaf = find_leaf_node(key);

    if (leaf == nullptr) {
        return std::nullopt; // Tree is empty or error in find_leaf_node
    }

    // In the leaf node, find the potential index of the key.
    // find_key_index returns the first index i such that keys[i] >= key.
    int key_idx_in_leaf = leaf->find_key_index(key);

    // Check if the key is found at that index and the index is valid.
    if (key_idx_in_leaf < leaf->n && leaf->keys[key_idx_in_leaf] == key) {
        // Key found
        return leaf->values[key_idx_in_leaf];
    } else {
        // Key not found
        return std::nullopt;
    }
}

// Public Method: remove
template<typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::remove(KeyType key) {
    if (root == nullptr) {
        std::cout << "Tree is empty. Cannot remove key " << key << std::endl;
        return;
    }

    // child_idx_in_parent is not strictly applicable for the root, can pass -1 or special value.
    // Or, the recursive function can handle parent_node == nullptr.
    remove_recursive(root, key, nullptr, -1);

    // After recursive call, root might need adjustment
    if (root != nullptr) { // Check if root was deleted (e.g. last key removed)
        if (root->n == 0) { // Root has no keys
            if (root->is_leaf) { // Root is a leaf and has 0 keys
                std::cout << "Tree became empty after removing key " << key << std::endl;
                delete root;
                root = nullptr;
            } else { // Root is internal and has 0 keys
                     // This means its last child was promoted to be the new root.
                     // (or its children were merged and it only has one child left)
                if (root->children.empty()) { // Should not happen if n=0 and not leaf, but defensive
                    delete root;
                    root = nullptr;
                } else if (root->children.size() == 1) { // Standard B-Tree root shrinkage
                    BPlusTreeNode<KeyType, ValueType>* old_root = root;
                    root = root->children[0];
                    root->parent = nullptr; // New root has no parent
                    std::cout << "Root height decreased after removing key " << key << std::endl;
                    // Need to ensure old_root's children vector is cleared if it's just a shell
                    old_root->children.clear(); // Avoid double deletion of the new root node
                    delete old_root;
                }
                // If root->n == 0 but children.size() > 1, it's an inconsistent state for B+ tree root.
                // This implies a key should have been present.
            }
        }
    }
}


// Stub for the recursive remove function. Will be implemented in parts.
template<typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::remove_recursive(
    BPlusTreeNode<KeyType, ValueType>* current_node,
    KeyType key_to_delete,
    BPlusTreeNode<KeyType, ValueType>* parent_node,
    int child_idx_in_parent) {
    // For now, just a placeholder. Actual implementation will be complex.
    // Minimum number of keys a non-root leaf node must have.
    // Min keys in leaf (non-root): ceil((ORDER-1)/2), which is ORDER/2 using integer division.
    //   ORDER=3 (max_keys=2): min_leaf=1
    //   ORDER=4 (max_keys=3): min_leaf=2
    //   ORDER=5 (max_keys=4): min_leaf=2
    const int MIN_LEAF_KEYS = ORDER / 2;

    // Min keys in internal node (non-root): ceil(ORDER/2)-1.
    //   ORDER=3 (max_children=3): min_children=2, min_keys=1
    //   ORDER=4 (max_children=4): min_children=2, min_keys=1
    //   ORDER=5 (max_children=5): min_children=3, min_keys=2
    const int MIN_INTERNAL_KEYS = (ORDER % 2 == 0) ? (ORDER / 2) - 1 : (ORDER / 2); // This is ceil(ORDER/2)-1

    if (current_node->is_leaf) {
        int key_idx = current_node->find_key_index(key_to_delete);

        if (key_idx < current_node->n && current_node->keys[key_idx] == key_to_delete) {
            // Key found, remove it
            current_node->remove_key_value_pair_from_leaf(key_idx);
            std::cout << "  Key " << key_to_delete << " removed from leaf." << std::endl;

            // Check for underflow, but not if it's the root
            if (parent_node != nullptr && current_node->n < MIN_LEAF_KEYS) {
                std::cout << "  Underflow in leaf node after removing " << key_to_delete << ". Current keys: " << current_node->n << ", Min keys: " << MIN_LEAF_KEYS << std::endl;

                BPlusTreeNode<KeyType, ValueType>* prev_sibling = (child_idx_in_parent > 0) ? parent_node->children[child_idx_in_parent - 1] : nullptr;
                BPlusTreeNode<KeyType, ValueType>* next_sibling = (child_idx_in_parent < parent_node->n) ? parent_node->children[child_idx_in_parent + 1] : nullptr;

                if (prev_sibling && prev_sibling->n > MIN_LEAF_KEYS) {
                    std::cout << "  Attempting to borrow from previous leaf sibling." << std::endl;
                    current_node->borrow_from_prev_leaf(prev_sibling, parent_node, child_idx_in_parent - 1);
                } else if (next_sibling && next_sibling->n > MIN_LEAF_KEYS) {
                    std::cout << "  Attempting to borrow from next leaf sibling." << std::endl;
                    current_node->borrow_from_next_leaf(next_sibling, parent_node, child_idx_in_parent);
                } else if (prev_sibling) { // Must merge with previous
                    std::cout << "  Attempting to merge with previous leaf sibling." << std::endl;
                    // prev_sibling merges current_node. current_node will be deleted.
                    // The key from parent (parent_node->keys[child_idx_in_parent - 1]) comes down into merged node.
                    // Parent needs to remove that key and current_node's child pointer.
                    current_node->merge_into_prev_leaf(prev_sibling, parent_node, child_idx_in_parent - 1);
                    // TODO: Parent needs to remove key at (child_idx_in_parent - 1) and child at child_idx_in_parent.
                    // TODO: Delete current_node.
                    // TODO: Recursively fix parent if it underflows. This is the complex part.
                    // For now, we assume merge_into_prev_leaf might adjust parent. If not, this needs more work.
                } else if (next_sibling) { // Must merge with next
                    std::cout << "  Attempting to merge with next leaf sibling." << std::endl;
                    // current_node merges next_sibling. next_sibling will be deleted.
                    // The key from parent (parent_node->keys[child_idx_in_parent]) comes down.
                    // Parent needs to remove that key and next_sibling's child pointer.
                    current_node->merge_next_leaf_into_current(next_sibling, parent_node, child_idx_in_parent);
                    // TODO: Parent needs to remove key at child_idx_in_parent and child at child_idx_in_parent + 1.
                    // TODO: Delete next_sibling.
                    // TODO: Recursively fix parent.
                }
                // If parent underflows due to merge, that's handled when remove_recursive returns to parent level.
            }
        } else {
            std::cout << "  Key " << key_to_delete << " not found in leaf." << std::endl;
        }
    } else { // Internal node
        int key_search_idx = current_node->find_key_index(key_to_delete);
        int child_descent_idx; // Index of the child subtree we will recurse into / has been processed

        if (key_search_idx < current_node->n && current_node->keys[key_search_idx] == key_to_delete) {
            // Key to delete is present in this internal node.
            // Replace with successor and delete successor from child.
            std::cout << "  Key " << key_to_delete << " found in internal node. Replacing with successor." << std::endl;
            KeyType successor_key = current_node->get_successor_from_child_subtree(key_search_idx + 1);
            current_node->keys[key_search_idx] = successor_key;
            child_descent_idx = key_search_idx + 1; // Recurse into the child from which successor was taken
            remove_recursive(current_node->children[child_descent_idx], successor_key, current_node, child_descent_idx);
        } else {
            // Key to delete is not in this internal node. Descend normally.
            child_descent_idx = key_search_idx; // find_key_index gives the correct child index
            remove_recursive(current_node->children[child_descent_idx], key_to_delete, current_node, child_descent_idx);
        }

        // After recursive call, check if the child that was processed needs underflow handling
        // (unless current_node is root and child_that_was_processed is the new root after tree shrinkage,
        // but root shrinkage is handled at the top level BPlusTree::remove)
        BPlusTreeNode<KeyType, ValueType>* child_that_was_processed = current_node->children[child_descent_idx];

        // Check if child_that_was_processed might have been deleted (due to merge)
        // This check is tricky. The parent (current_node) would have its children list modified by merge.
        // The child_descent_idx might become invalid if a merge with previous occurred.
        // For now, assume child_that_was_processed pointer is still valid if not merged, or points to merged node.
        // A better way: child_that_was_processed is identified by its original index. If it was merged into prev,
        // then current_node->children[child_descent_idx-1] is the merged node.
        // If it merged next into itself, current_node->children[child_descent_idx] is the merged node.
        // This part of the logic needs to be robust. The merge methods adjust the parent.

        // Let's assume for now that if a merge occurred, the child at child_descent_idx is the one that remains (or its prev sibling).
        // The number of keys in parent (current_node) might also have changed if a merge happened.

        // We need to re-evaluate the child reference and its underflow condition, especially if child_descent_idx was affected by a merge.
        // A simple approach: check all children. But that's inefficient.
        // The merge methods should make the parent node (current_node here) directly remove the key and child.
        // So, current_node->n would reflect this change immediately.

        // If current_node itself is underflowing (and is not root), it will be handled when this call returns to its parent.
        // We only handle underflow of current_node's *children* here.
        // The child_idx_in_parent passed to this call is for current_node in ITS parent.

        // The child that was processed is current_node->children[child_descent_idx]
        // OR, if that child was merged into its previous sibling, then child_descent_idx effectively decreased by 1 for subsequent children.
        // This makes direct indexing after recursive call complex if merges alter parent's children list during child's recursive call.

        // Let's assume the merge methods in BPlusTreeNode correctly update the PARENT node's keys and children list.
        // This means current_node (as parent of child_that_was_processed) is already modified by the merge call
        // that happened deeper in recursion.
        // So, after remove_recursive returns, current_node's state (n, keys, children) is up-to-date.
        // We don't need to check a specific child for underflow; current_node's underflow is handled by ITS parent.

        // The TODOs in leaf merge about parent updates are critical.
        // If merge_into_prev_leaf is called on C by P:
        // C tells P_sibling to absorb C. P_sibling does.
        // Then P needs to remove key_between_P_C and ptr_to_C.
        // This removal from P is what we are doing here (current_node is P).

        // Let's re-evaluate the structure. remove_recursive(child, key, parent=current_node, child_idx_in_current_node)
        // If child underflows and merges, child calls current_node->(internal_merge_helper_on_parent)
        // No, the BPlusTreeNode merge methods are called ON the child/sibling.
        // They don't directly modify parent. They expect parent to be modified by remove_recursive.
        // This means after a merge, `remove_recursive` needs to update its `current_node` (which is parent of merged nodes).

        // This subtask's plan: "parent_node removes keys[parent_key_idx] and children[parent_key_idx + 1]"
        // This should be done by the node methods themselves or by remove_recursive immediately after merge.
        // The stubs for merge_xxx_internal already say "Parent node loses a key and a child pointer... handled by the caller".
        // This means the `remove_recursive` call that *triggered* the merge (by calling remove_recursive on its child)
        // is responsible for updating itself (the parent of the merged nodes).

        // So, if `remove_recursive(child_node_to_descend, ...)` resulted in a merge of `child_node_to_descend`:
        // `current_node` (which is `parent_node` for that call) needs to be updated.
        // This update happens *within this current invocation* of `remove_recursive`.
        // The handling of child underflow is below.

        // If a child of current_node underflowed and was handled (borrow/merge):
        // The number of keys 'n' in current_node might have changed if a merge occurred among its children.
        // Now, check if current_node itself is underflowing (and is not the tree's root).
        // This is handled when this function returns, by its caller.
        // So, the logic for handling child underflow is what's needed here.

        // The child_that_was_processed is at current_node->children[child_descent_idx]
        // *UNLESS* it merged with its previous sibling. In that case, it was deleted, and
        // effectively all children at indices > child_descent_idx shifted left.
        // And current_node->n (number of keys) would have decreased by 1.
        // This is why passing child_idx_in_parent by reference was considered.
        // For now, let's assume child_descent_idx remains the point of interest or child_descent_idx-1 if merge with prev.

        // Let's simplify the post-recursion check. A child might have become underfull.
        // We need to iterate through children of current_node IF a merge happened that modified current_node.
        // Or, more directly, the child that was processed (current_node->children[child_descent_idx]) is checked.
        // If it was deleted (merged into prev), then we should check the merged node (current_node->children[child_descent_idx-1]).

        // This recursive structure is hard. A common way is for remove_recursive to return a "state"
        // (e.g., " خوراک ", "underflow_handled_by_borrow", "underflow_handled_by_merge_i_was_deleted").
        // Given current structure, we'll assume `child_that_was_processed` is the node to check,
        // and its index `child_descent_idx` is still valid *relative to current_node's state before this check*.

        // If child_that_was_processed was deleted (e.g. merged into its previous sibling),
        // then current_node->children list is shorter.
        // The node methods `merge_into_prev_internal` and `merge_next_internal_into_current`
        // state that the CALLER (this `remove_recursive` instance, acting as parent)
        // is responsible for removing the key/child from parent and deleting the node.

        // This means the underflow handling for current_node's child happens here,
        // *including* modifying current_node if a merge occurs between its children.
        if (child_that_was_processed->n < (child_that_was_processed->is_leaf ? MIN_LEAF_KEYS : MIN_INTERNAL_KEYS) && child_that_was_processed != root) { // Check if child underflowed
             std::cout << "  Child (idx " << child_descent_idx << ") of internal node underflowed. Attempting to fix." << std::endl;
            BPlusTreeNode<KeyType, ValueType>* prev_sibling_of_child = (child_descent_idx > 0) ? current_node->children[child_descent_idx - 1] : nullptr;
            BPlusTreeNode<KeyType, ValueType>* next_sibling_of_child = (child_descent_idx < current_node->n) ? current_node->children[child_descent_idx + 1] : nullptr; // current_node->n is num_keys, so num_children is n+1

            int min_keys_for_child_type = child_that_was_processed->is_leaf ? MIN_LEAF_KEYS : MIN_INTERNAL_KEYS;

            if (prev_sibling_of_child && prev_sibling_of_child->n > min_keys_for_child_type) {
                std::cout << "  Child borrowing from its previous internal/leaf sibling." << std::endl;
                if (child_that_was_processed->is_leaf) child_that_was_processed->borrow_from_prev_leaf(prev_sibling_of_child, current_node, child_descent_idx - 1);
                else child_that_was_processed->borrow_from_prev_internal(prev_sibling_of_child, current_node, child_descent_idx - 1);
            } else if (next_sibling_of_child && next_sibling_of_child->n > min_keys_for_child_type) {
                std::cout << "  Child borrowing from its next internal/leaf sibling." << std::endl;
                 if (child_that_was_processed->is_leaf) child_that_was_processed->borrow_from_next_leaf(next_sibling_of_child, current_node, child_descent_idx);
                 else child_that_was_processed->borrow_from_next_internal(next_sibling_of_child, current_node, child_descent_idx);
            } else if (prev_sibling_of_child) { // Must merge with previous
                std::cout << "  Child merging with its previous internal/leaf sibling." << std::endl;
                if (child_that_was_processed->is_leaf) child_that_was_processed->merge_into_prev_leaf(prev_sibling_of_child, current_node, child_descent_idx - 1);
                else child_that_was_processed->merge_into_prev_internal(prev_sibling_of_child, current_node, child_descent_idx - 1);

                // Parent (current_node) must remove key and child pointer
                current_node->remove_key_from_internal_node(child_descent_idx - 1, child_descent_idx); // key_idx, child_idx
                delete child_that_was_processed;
                // child_descent_idx effectively becomes child_descent_idx-1 for parent's perspective on children array
            } else if (next_sibling_of_child) { // Must merge with next
                std::cout << "  Child merging with its next internal/leaf sibling." << std::endl;
                if (child_that_was_processed->is_leaf) child_that_was_processed->merge_next_leaf_into_current(next_sibling_of_child, current_node, child_descent_idx);
                else child_that_was_processed->merge_next_internal_into_current(next_sibling_of_child, current_node, child_descent_idx);

                // Parent (current_node) must remove key and child pointer
                current_node->remove_key_from_internal_node(child_descent_idx, child_descent_idx + 1); // key_idx, child_idx
                delete next_sibling_of_child;
            }
        }
    }
}


// Private Helper: find_leaf_node
template<typename KeyType, typename ValueType>
BPlusTreeNode<KeyType, ValueType>* BPlusTree<KeyType, ValueType>::find_leaf_node(KeyType key) {
    if (root == nullptr) {
        return nullptr;
    }

    BPlusTreeNode<KeyType, ValueType>* current_node = root;
    while (!current_node->is_leaf) {
        // find_key_index in an internal node returns the index of the first key
        // that is greater than or equal to the given key.
        // This means the key should be in the subtree pointed to by children[index].
        int child_idx = current_node->find_key_index(key);

        // If key is greater than all keys in current_node, find_key_index returns n.
        // The child pointer is children[n].
        // If key is smaller than all keys, find_key_index returns 0. child is children[0].
        // If key is between keys[i-1] and keys[i], find_key_index returns i. child is children[i].
        // This logic is consistent with typical B-Tree/B+Tree search.

        if (child_idx < current_node->n && current_node->keys[child_idx] == key && !current_node->is_leaf) {
            // In B+ tree, if key is found in internal node, it acts as a separator.
            // We need to go to the right child of this key to find the actual data in a leaf.
            // So, children[child_idx + 1] is the correct path if keys[child_idx] == key.
            // However, find_key_index returns the index of the first key >= given key.
            // If keys[child_idx] == key, then we want children[child_idx+1]
            // If keys[child_idx] > key, then we want children[child_idx]
            // The current find_key_index implementation is:
            //   while (index < n && keys[index] < key) { index++; } return index;
            // If keys[child_idx] == key, it means keys[child_idx] is the first key >= key. We take children[child_idx+1].
            // If keys[child_idx] > key, it means keys[child_idx] is the first key >= key. We take children[child_idx].
            // This seems to conflict. Let's re-evaluate find_key_index for internal nodes.

            // Standard B-Tree search:
            // 1. Find i such that keys[i-1] <= key < keys[i]. Descend to children[i].
            // 2. If key >= keys[n-1], descend to children[n].
            // The current find_key_index:
            //   - if key < keys[0], returns 0. current_node->children[0] is correct.
            //   - if keys[i-1] <= key < keys[i], it means keys[i-1] < key, so loop continues.
            //     Eventually index points to i (first key >= key). current_node->children[i] is correct.
            //   - if key == keys[i], it returns i. current_node->children[i] is NOT where key is if B+ rules are strict (key only in leaf).
            //     For B+ tree, if key == keys[i] in internal node, key is *repeated* in leaf. Path is children[i+1].
            // So, if current_node->keys[child_idx] == key, we should use child_idx + 1.
             if (child_idx < current_node->n && current_node->keys[child_idx] == key) {
                 current_node = current_node->children[child_idx + 1];
             } else {
                 current_node = current_node->children[child_idx];
             }
        } else {
             current_node = current_node->children[child_idx];
        }


        if (current_node == nullptr) {
            // This should ideally not happen in a well-formed B+ Tree unless empty.
            // Or if find_key_index logic has an issue.
            // For now, let's assume valid structure and non-null children.
            // Consider adding an error or assertion here.
            return nullptr;
        }
    }
    return current_node; // This is a leaf node
}


// Explicit template instantiations
// Add instantiations for common KeyType and ValueType combinations used in the project.
template class BPlusTree<int, int>;
template class BPlusTree<int, double>;
template class BPlusTree<int, std::string>;
template class BPlusTree<std::string, std::string>; // For string tests
template class BPlusTree<std::string, int>; // For string key, int value tests
// Add more as required by the actual usage.
// For example, if using char keys and int values:
// template class BPlusTree<char, int>;
