// To compile and run this demo:
// g++ -std=c++17 bplus_tree_node.cpp bplus_tree.cpp bplus_tree_demo.cpp -o bplus_tree_demo_runner -Wall -Wextra -g
// ./bplus_tree_demo_runner

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <utility> // For std::pair

#include "bplus_tree.h"
#include "bplus_tree_node.h" // For ORDER constant

int main() {
    std::cout << "B+ Tree Demonstration (ORDER = " << ORDER << ")" << std::endl;
    std::cout << "===================================" << std::endl;

    // --- Demonstrating BPlusTree<int, std::string> ---
    std::cout << "\n--- Demonstrating BPlusTree<int, std::string> ---" << std::endl;
    BPlusTree<int, std::string> tree_int_string;

    // Insertions
    std::cout << "\nInserting key-value pairs..." << std::endl;
    tree_int_string.insert(10, "apple");
    tree_int_string.insert(20, "banana");
    tree_int_string.insert(5, "orange");   // Should trigger leaf split if ORDER=3 (10,20 full -> 5,10,20 -> split)
                                        // Expected (ORDER=3): Root:[20], L:[5,10], R:[20]
    tree_int_string.insert(15, "grape");   // Goes to L:[5,10]. Becomes [5,10,15]. Splits. L_L:[5,10], L_R:[15]. Promotes 15.
                                        // Root:[20] inserts 15 -> Root:[15,20]. Children: L_L, L_R, R
    tree_int_string.insert(25, "melon");   // Goes to R:[20]. Becomes [20,25].
    tree_int_string.insert(30, "peach");   // Goes to R:[20,25]. Becomes [20,25,30]. Splits. R_L:[20,25], R_R:[30]. Promotes 30.
                                        // Root:[15,20] inserts 30 -> Root:[15,20,30]. Root splits!
                                        // New Root: [20]. Children: L:[15] (with children L_L, L_R), R:[30] (with children R_L, R_R)

    // Traversal/Output after Insertions
    std::cout << "\nTree<int, string> after insertions (BPlusTreeNode::traverse output):" << std::endl;
    tree_int_string.traverse();

    std::cout << "\nTree<int, string> leaf key-value pairs (iterating leaves):" << std::endl;
    BPlusTreeNode<int, std::string>* current_leaf_is = tree_int_string.get_first_leaf();
    while (current_leaf_is) {
        std::cout << "Leaf: ";
        for (int i = 0; i < current_leaf_is->n; ++i) { // Changed i to int
            std::cout << "(" << current_leaf_is->keys[i] << ": " << current_leaf_is->values[i] << ") ";
        }
        std::cout << "-> ";
        current_leaf_is = current_leaf_is->next_leaf;
    }
    std::cout << "nullptr" << std::endl;

    // Searches
    std::cout << "\nSearching for key 15:" << std::endl;
    std::optional<std::string> val_is = tree_int_string.search(15);
    if (val_is) {
        std::cout << "Found! Value: " << *val_is << std::endl;
    } else {
        std::cout << "Not found." << std::endl;
    }

    std::cout << "Searching for key 100 (absent):" << std::endl;
    val_is = tree_int_string.search(100);
    if (val_is) {
        std::cout << "Found! Value: " << *val_is << std::endl;
    } else {
        std::cout << "Not found." << std::endl;
    }

    // Deletions
    std::cout << "\nDeleting key 10..." << std::endl;
    tree_int_string.remove(10);
    std::cout << "Tree state after deleting 10 (leaf scan):" << std::endl;
    current_leaf_is = tree_int_string.get_first_leaf();
    while (current_leaf_is) {
        std::cout << "Leaf: ";
        for (int i = 0; i < current_leaf_is->n; ++i) { // Changed i to int
            std::cout << "(" << current_leaf_is->keys[i] << ": " << current_leaf_is->values[i] << ") ";
        }
        std::cout << "-> ";
        current_leaf_is = current_leaf_is->next_leaf;
    }
    std::cout << "nullptr" << std::endl;
    // std::cout << "Tree structure after deleting 10 (traverse):" << std::endl;
    // tree_int_string.traverse();


    std::cout << "\nDeleting key 5..." << std::endl;
    tree_int_string.remove(5);
    std::cout << "Tree state after deleting 5 (leaf scan):" << std::endl;
    current_leaf_is = tree_int_string.get_first_leaf();
    while (current_leaf_is) {
        std::cout << "Leaf: ";
        for (int i = 0; i < current_leaf_is->n; ++i) { // Changed i to int
            std::cout << "(" << current_leaf_is->keys[i] << ": " << current_leaf_is->values[i] << ") ";
        }
        std::cout << "-> ";
        current_leaf_is = current_leaf_is->next_leaf;
    }
    std::cout << "nullptr" << std::endl;
    // std::cout << "Tree structure after deleting 5 (traverse):" << std::endl;
    // tree_int_string.traverse();


    // --- Demonstrating BPlusTree<std::string, int> ---
    std::cout << "\n\n--- Demonstrating BPlusTree<std::string, int> ---" << std::endl;
    BPlusTree<std::string, int> tree_string_int;

    // Insertions
    std::cout << "\nInserting key-value pairs..." << std::endl;
    tree_string_int.insert("gamma", 300);
    tree_string_int.insert("alpha", 100);
    tree_string_int.insert("beta", 200);  // alpha, beta, gamma -> Root: beta, L:[alpha], R:[beta,gamma]
    tree_string_int.insert("delta", 400); // Goes to R. R:[beta,gamma,delta] -> R_L:[beta,gamma], R_R:[delta]. Promotes delta.
                                       // Root: beta inserts delta -> Root:[beta,delta]. Children: L, R_L, R_R

    // Traversal/Output after Insertions
    std::cout << "\nTree<string, int> after insertions (BPlusTreeNode::traverse output):" << std::endl;
    tree_string_int.traverse();

    std::cout << "\nTree<string, int> leaf key-value pairs (iterating leaves):" << std::endl;
    BPlusTreeNode<std::string, int>* current_leaf_si = tree_string_int.get_first_leaf();
    while (current_leaf_si) {
        std::cout << "Leaf: ";
        for (int i = 0; i < current_leaf_si->n; ++i) { // Changed i to int
            std::cout << "(" << current_leaf_si->keys[i] << ": " << current_leaf_si->values[i] << ") ";
        }
        std::cout << "-> ";
        current_leaf_si = current_leaf_si->next_leaf;
    }
    std::cout << "nullptr" << std::endl;

    std::cout << "\nDemo finished." << std::endl;
    return 0;
}
