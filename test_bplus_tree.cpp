// Compile with: g++ -std=c++17 bplus_tree_node.cpp bplus_tree.cpp test_bplus_tree.cpp -o test_bplus_tree_runner -Wall -Wextra -g -fsanitize=address

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <utility> // For std::pair
#include <algorithm> // For std::equal
#include <cassert>

#include "bplus_tree.h" // This should also make bplus_tree_node.h available

// Test Helper: Print test result
void print_test_result(const std::string& test_name, bool success) {
    std::cout << "Test: " << test_name << " - " << (success ? "PASSED" : "FAILED") << std::endl;
}

// Test Helper: Get all leaf key-value pairs in order
template<typename KeyType, typename ValueType>
std::vector<std::pair<KeyType, ValueType>> get_all_leaf_keys_values(BPlusTree<KeyType, ValueType>& tree) {
    std::vector<std::pair<KeyType, ValueType>> pairs;
    BPlusTreeNode<KeyType, ValueType>* current_leaf = tree.get_first_leaf();
    while (current_leaf) {
        for (int i = 0; i < current_leaf->n; ++i) {
            pairs.push_back({current_leaf->keys[i], current_leaf->values[i]});
        }
        current_leaf = current_leaf->next_leaf;
    }
    return pairs;
}

// Test Case: Empty Tree
void test_empty_tree() {
    std::string test_name = "test_empty_tree";
    bool success = true;
    BPlusTree<int, int> tree;

    if (tree.search(10).has_value()) {
        success = false;
        std::cerr << test_name << ": Search on empty tree returned a value." << std::endl;
    }

    std::vector<std::pair<int, int>> values = get_all_leaf_keys_values(tree);
    if (!values.empty()) {
        success = false;
        std::cerr << test_name << ": get_all_leaf_keys_values on empty tree returned non-empty vector." << std::endl;
    }

    // tree.traverse(); // Manual check: should print "Tree is empty."
    print_test_result(test_name, success);
}

// Test Case: Single Insert and Search
void test_single_insert_search() {
    std::string test_name = "test_single_insert_search";
    bool success = true;
    BPlusTree<int, int> tree;

    tree.insert(10, 100);

    std::optional<int> val = tree.search(10);
    if (!val.has_value() || val.value() != 100) {
        success = false;
        std::cerr << test_name << ": Search for 10 failed or returned wrong value." << std::endl;
    }

    if (tree.search(20).has_value()) {
        success = false;
        std::cerr << test_name << ": Search for 20 (absent) returned a value." << std::endl;
    }

    std::vector<std::pair<int, int>> expected_pairs = {{10, 100}};
    std::vector<std::pair<int, int>> actual_pairs = get_all_leaf_keys_values(tree);
    if (actual_pairs != expected_pairs) {
        success = false;
        std::cerr << test_name << ": get_all_leaf_keys_values returned unexpected pairs." << std::endl;
    }

    // tree.traverse(); // Manual check
    print_test_result(test_name, success);
}

// Test Case: Multiple Inserts (Leaf Split with ORDER=3)
void test_multiple_inserts_leaf_split_ORDER3() {
    std::string test_name = "test_multiple_inserts_leaf_split_ORDER3 (ORDER=3)";
    bool success = true;
    BPlusTree<int, int> tree; // ORDER is 3 from bplus_tree_node.h

    tree.insert(10, 100);
    tree.insert(20, 200); // Leaf: [10,20] (full)

    std::vector<std::pair<int, int>> expected_pairs_1 = {{10, 100}, {20, 200}};
    std::vector<std::pair<int, int>> actual_pairs_1 = get_all_leaf_keys_values(tree);
    if (actual_pairs_1 != expected_pairs_1) {
        success = false;
        std::cerr << test_name << " (step 1): get_all_leaf_keys_values returned:" << std::endl;
        for(const auto& p : actual_pairs_1) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;
    }
    assert(tree.search(10).value_or(-1) == 100);
    assert(tree.search(20).value_or(-1) == 200);


    // Insert 5, causing split. Expected structure: Root [20], L:[5,10], R:[20]
    // (Based on split_leaf_node: overfull [5,10,20] -> Left gets ceil(3/2)=2 -> [5,10]. Right gets floor(3/2)=1 -> [20]. ParentKey: 20)
    tree.insert(5, 50);

    std::vector<std::pair<int, int>> expected_pairs_2 = {{5, 50}, {10, 100}, {20, 200}};
    std::vector<std::pair<int, int>> actual_pairs_2 = get_all_leaf_keys_values(tree);
     if (actual_pairs_2 != expected_pairs_2) {
        success = false;
        std::cerr << test_name << " (step 2 - after inserting 5): get_all_leaf_keys_values returned:" << std::endl;
        for(const auto& p : actual_pairs_2) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;
        std::cerr << "Expected:" << std::endl;
        for(const auto& p : expected_pairs_2) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;

    }
    assert(tree.search(5).value_or(-1) == 50);
    assert(tree.search(10).value_or(-1) == 100);
    assert(tree.search(20).value_or(-1) == 200);

    // Manual check of structure via traverse (or debugger) would be needed here for root key.
    // For now, checking leaf values is the primary automated check.
    // std::cout << "Traverse after inserting 5, 10, 20 (ORDER=3):" << std::endl;
    // tree.traverse();

    print_test_result(test_name, success);
}

// Test Case: Multiple Inserts (Internal Split with ORDER=3)
void test_multiple_inserts_internal_split_ORDER3() {
    std::string test_name = "test_multiple_inserts_internal_split_ORDER3 (ORDER=3)";
    bool success = true;
    BPlusTree<int, int> tree;

    // Setup: Root [20], L:[5,10], R:[20]
    tree.insert(10, 100);
    tree.insert(20, 200);
    tree.insert(5, 50);

    // Insert (15,150). Goes to L:[5,10]. L becomes [5,10,15]. Splits.
    // L_left:[5,10], L_right:[15]. ParentKey:15.
    // Root [20] needs to insert key 15. Becomes [15,20].
    // Children: L_left:[5,10], L_right:[15], R:[20].
    tree.insert(15, 150);

    std::vector<std::pair<int, int>> expected_pairs = {{5, 50}, {10, 100}, {15, 150}, {20, 200}};
    std::vector<std::pair<int, int>> actual_pairs = get_all_leaf_keys_values(tree);
    if (actual_pairs != expected_pairs) {
        success = false;
        std::cerr << test_name << ": get_all_leaf_keys_values returned:" << std::endl;
        for(const auto& p : actual_pairs) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;
        std::cerr << "Expected:" << std::endl;
        for(const auto& p : expected_pairs) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;
    }
    assert(tree.search(5).value_or(-1) == 50);
    assert(tree.search(10).value_or(-1) == 100);
    assert(tree.search(15).value_or(-1) == 150);
    assert(tree.search(20).value_or(-1) == 200);

    // std::cout << "Traverse after inserting 15 (ORDER=3):" << std::endl;
    // tree.traverse();
    print_test_result(test_name, success);
}

// Test Case: Root Split (ORDER=3)
void test_root_split_ORDER3() {
    std::string test_name = "test_root_split_ORDER3 (ORDER=3)";
    // bool success = true;
    BPlusTree<int, int> tree;
    // Setup from previous test: Root [15,20]. Children: C1:[5,10], C2:[15], C3:[20].
    tree.insert(10, 100); tree.insert(20, 200); tree.insert(5, 50); tree.insert(15, 150);

    // Insert (25,250). Goes to C3:[20]. C3 becomes [20,25]. No split of C3.
    tree.insert(25, 250);
    // Current state: Root [15,20]. Children: C1:[5,10], C2:[15], C3:[20,25].

    // Insert (30,300). Goes to C3 [20,25]. C3 becomes [20,25,30]. Splits.
    // C3_left:[20,25], C3_right:[30]. ParentKey:30.
    // Root [15,20] needs to insert key 30. Becomes [15,20,30]. Overfull internal node. Splits.
    // Median key 20 goes to New Root.
    // New Root: [20].
    // Left Child of New Root: keys=[15], children=[C1:[5,10], C2:[15]].
    // Right Child of New Root: keys=[30], children=[C3_left:[20,25], C3_right:[30]].
    tree.insert(30, 300);

    std::vector<std::pair<int, int>> expected_pairs = {
        {5, 50}, {10, 100}, {15, 150}, {20, 200}, {25, 250}, {30, 300}
    };
    std::vector<std::pair<int, int>> actual_pairs = get_all_leaf_keys_values(tree);
    bool success = (actual_pairs == expected_pairs);
    if (!success) {
        std::cerr << test_name << ": get_all_leaf_keys_values returned:" << std::endl;
        for(const auto& p : actual_pairs) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;
        std::cerr << "Expected:" << std::endl;
        for(const auto& p : expected_pairs) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;
    }
    assert(tree.search(5).value_or(-1) == 50);
    assert(tree.search(10).value_or(-1) == 100);
    assert(tree.search(15).value_or(-1) == 150);
    assert(tree.search(20).value_or(-1) == 200);
    assert(tree.search(25).value_or(-1) == 250);
    assert(tree.search(30).value_or(-1) == 300);

    // std::cout << "Traverse after inserting 30 (ORDER=3) - should cause root split:" << std::endl;
    // tree.traverse();
    print_test_result(test_name, success);
}

// Test Case: Simple Deletion (No Underflow)
void test_simple_delete_no_underflow_ORDER3() {
    std::string test_name = "test_simple_delete_no_underflow_ORDER3 (ORDER=3)";
    BPlusTree<int, int> tree;
    tree.insert(5,50); tree.insert(10,100); tree.insert(15,150); tree.insert(20,200); tree.insert(25,250);
    // Structure: Root [15,20], C1[5,10], C2[15], C3[20,25]

    tree.remove(10); // Remove from C1:[5,10] -> C1:[5] (still one key, min for ORDER=3 is 1)

    std::vector<std::pair<int, int>> expected_pairs = {{5, 50}, {15, 150}, {20, 200}, {25, 250}};
    std::vector<std::pair<int, int>> actual_pairs = get_all_leaf_keys_values(tree);
    bool success = (actual_pairs == expected_pairs);
     if (!success) {
        std::cerr << test_name << ": get_all_leaf_keys_values after deleting 10:" << std::endl;
        for(const auto& p : actual_pairs) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;
    }
    assert(tree.search(10).has_value() == false);
    assert(tree.search(5).value_or(-1) == 50);

    // std::cout << "Traverse after deleting 10 (ORDER=3):" << std::endl;
    // tree.traverse();
    print_test_result(test_name, success);
}

// Test Case: Delete to Empty Tree
void test_delete_to_empty_ORDER3() {
    std::string test_name = "test_delete_to_empty_ORDER3 (ORDER=3)";
    BPlusTree<int, int> tree;
    tree.insert(10, 100);
    tree.remove(10);

    bool success = (tree.get_first_leaf() == nullptr); // Root should be null
    if (!tree.search(10).has_value() && success) {
        // success is true
    } else {
        success = false;
        std::cerr << test_name << ": Tree not empty or search still finds value." << std::endl;
    }
    std::vector<std::pair<int, int>> actual_pairs = get_all_leaf_keys_values(tree);
     if (!actual_pairs.empty()) {
        success = false;
        std::cerr << test_name << ": get_all_leaf_keys_values not empty." << std::endl;
    }

    // tree.traverse(); // Should print "Tree is empty"
    print_test_result(test_name, success);
}

// Test Case: String Keys and Values
void test_string_keys_values() {
    std::string test_name = "test_string_keys_values (ORDER=3)";
    BPlusTree<std::string, std::string> tree;
    tree.insert("banana", "yellow");
    tree.insert("apple", "red");
    tree.insert("orange", "orange_color"); // Leaf split: apple,banana - orange. Root: "orange" L:[apple,banana] R:[orange]
                                     // (Split: [apple,banana,orange] -> L:[apple,banana] R:[orange]. Parent: "orange")

    bool success = true;
    if (tree.search("apple").value_or("") != "red") success = false;
    if (tree.search("banana").value_or("") != "yellow") success = false;
    if (tree.search("orange").value_or("") != "orange_color") success = false;
    if (!success) std::cerr << test_name << ": Search failed for one or more string keys." << std::endl;

    std::vector<std::pair<std::string, std::string>> expected_pairs = {
        {"apple", "red"}, {"banana", "yellow"}, {"orange", "orange_color"}
    };
    std::vector<std::pair<std::string, std::string>> actual_pairs = get_all_leaf_keys_values(tree);
    if (actual_pairs != expected_pairs) {
        success = false;
        std::cerr << test_name << ": get_all_leaf_keys_values for strings returned:" << std::endl;
        for(const auto& p : actual_pairs) { std::cerr << " (" << p.first << "," << p.second << ")"; } std::cerr << std::endl;
    }
    // tree.traverse();
    print_test_result(test_name, success);
}


int main() {
    std::cout << "Starting BPlusTree Tests..." << std::endl;
    // Note: ORDER is assumed to be 3 from bplus_tree_node.h for all ORDER=3 tests.

    test_empty_tree();
    test_single_insert_search();
    test_multiple_inserts_leaf_split_ORDER3();
    test_multiple_inserts_internal_split_ORDER3();
    test_root_split_ORDER3();
    test_simple_delete_no_underflow_ORDER3();
    test_delete_to_empty_ORDER3();
    test_string_keys_values();

    std::cout << "BPlusTree Tests Finished." << std::endl;
    return 0;
}
