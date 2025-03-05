#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <vector>
#include <string>

template <typename T>
class BTreeNode {
public:
    bool isLeaf;
    std::vector<T> keys;
    std::vector<BTreeNode<T>*> children;

    BTreeNode(bool leaf) : isLeaf(leaf) {}

    // 🛠️ Mark traverse as const
    void traverse() const {
        int i;
        for (i = 0; i < keys.size(); i++) {
            if (!isLeaf)
                children[i]->traverse();
            std::cout << " " << keys[i];
        }
        if (!isLeaf)
            children[i]->traverse();
    }

    // 🛠️ Add `t` as parameter
    void insertNonFull(const T& key, int t);
    void splitChild(int i, BTreeNode<T>* y, int t);
    bool remove(const T& key);
    std::vector<T> rangeSearch(const T& prefix) const;
};

template <typename T>
class BTree {
private:
    BTreeNode<T>* root;
    int t;  // Minimum degree

public:
    BTree(int _t) : root(nullptr), t(_t) {}

    void traverse() const {
        if (root) root->traverse();
    }

    void insert(const T& key);
    void remove(const T& key);
    std::vector<T> rangeSearch(const T& prefix) const;
};

// 🛠️ Insert key into B-Tree
template <typename T>
void BTree<T>::insert(const T& key) {
    if (!root) {
        root = new BTreeNode<T>(true);
        root->keys.push_back(key);
    } else {
        if (root->keys.size() == 2 * t - 1) {
            BTreeNode<T>* s = new BTreeNode<T>(false);
            s->children.push_back(root);
            s->splitChild(0, root, t);  // Pass `t` to splitChild
            int i = (s->keys[0] < key) ? 1 : 0;
            s->children[i]->insertNonFull(key, t);  // Pass `t` to insertNonFull
            root = s;
        } else {
            root->insertNonFull(key, t);  // Pass `t` to insertNonFull
        }
    }
}

// 🛠️ Remove key from B-Tree
template <typename T>
void BTree<T>::remove(const T& key) {
    if (!root) return;
    root->remove(key);
}

// 🛠️ Range search for prefix in B-Tree
template <typename T>
std::vector<T> BTree<T>::rangeSearch(const T& prefix) const {
    std::vector<T> results;
    if (root) {
        std::function<void(BTreeNode<T>*)> search = [&](BTreeNode<T>* node) {
            for (const T& key : node->keys) {
                if (key.find(prefix) == 0) results.push_back(key);
            }
            for (auto child : node->children) {
                if (child) search(child);
            }
        };
        search(root);
    }
    return results;
}

// 🛠️ Insert non-full with `t`
template <typename T>
void BTreeNode<T>::insertNonFull(const T& key, int t) {
    int i = keys.size() - 1;
    if (isLeaf) {
        keys.insert(keys.begin() + (i + 1), key);
    } else {
        while (i >= 0 && keys[i] > key) i--;
        if (children[i + 1]->keys.size() == 2 * t - 1) {
            splitChild(i + 1, children[i + 1], t);  // Pass `t` to splitChild
            if (keys[i + 1] < key) i++;
        }
        children[i + 1]->insertNonFull(key, t);  // Pass `t` to insertNonFull
    }
}

// 🛠️ Split child with `t`
template <typename T>
void BTreeNode<T>::splitChild(int i, BTreeNode<T>* y, int t) {
    BTreeNode<T>* z = new BTreeNode<T>(y->isLeaf);
    z->keys.insert(z->keys.begin(), y->keys.begin() + t, y->keys.end());
    if (!y->isLeaf)
        z->children.insert(z->children.begin(), y->children.begin() + t, y->children.end());

    y->keys.resize(t - 1);
    y->children.resize(t);

    children.insert(children.begin() + i + 1, z);
    keys.insert(keys.begin() + i, y->keys[t - 1]);
}

// 🛠️ Remove key (dummy implementation for now)
template <typename T>
bool BTreeNode<T>::remove(const T& key) {
    auto it = std::find(keys.begin(), keys.end(), key);
    if (it != keys.end()) {
        keys.erase(it);
        return true;
    }
    return false;
}

#endif
