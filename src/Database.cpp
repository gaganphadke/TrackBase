#include "/Users/gaganphadke/Versioning/versioned-db/include/Database.h"
#include "/Users/gaganphadke/Versioning/versioned-db/include/BTree.h"
#include <fstream>
#include <iostream>
#include <regex>
#include </opt/homebrew/Cellar/nlohmann-json/3.11.3/include/nlohmann/json.hpp>
#include <mutex>
#include <unordered_set>
#include <algorithm>
using json = nlohmann::json;

std::mutex dataMutex;  // Define the mutex globally if not defined elsewhere

std::unordered_map<std::string, std::set<Edge>> graph; 

// 🛠️ Insert a node
void Database::insertNode(const std::string& node) {
    if (!graph.count(node)) graph[node] = {};
}

// 🛠️ Insert an edge
void Database::insertEdge(const std::string& from, const std::string& to, int weight) {
    graph[from].insert({to, weight});
    graph[to].insert({from, weight});  // For undirected graphs
}

// 🛠️ BFS traversal
std::vector<std::string> Database::bfs(const std::string& start) {
    std::vector<std::string> result;
    std::queue<std::string> q;
    std::set<std::string> visited;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        auto node = q.front(); q.pop();
        result.push_back(node);
        for (const auto& edge : graph[node]) {
            if (!visited.count(edge.to)) {
                visited.insert(edge.to);
                q.push(edge.to);
            }
        }
    }
    return result;
}

// 🛠️ DFS traversal
std::vector<std::string> Database::dfs(const std::string& start) {
    std::vector<std::string> result;
    std::stack<std::string> s;
    std::set<std::string> visited;

    s.push(start);
    while (!s.empty()) {
        auto node = s.top(); s.pop();
        if (visited.count(node)) continue;
        visited.insert(node);
        result.push_back(node);
        for (const auto& edge : graph[node]) {
            s.push(edge.to);
        }
    }
    return result;
}

// 🛠️ Constructor with B-Tree Initialization
Database::Database(const std::string& filename) 
    : filename(filename), keyIndex(3), valueIndex(3) {}

// 🛠️ Load data from file and build B-Tree indices
bool Database::load() {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) return false;

    try {
        json j;
        inFile >> j;

        std::lock_guard<std::mutex> lock(dataMutex);
        data.clear();

        for (auto& [key, value] : j.items()) {
            if (value.is_string()) {
                data[key] = value.get<std::string>();
            } else {
                data[key] = value.dump();
            }
        }

        buildBTreeIndices();  // Build B-Tree indices on load

        for (const auto& [key, value] : data) {
            updateIndices(key, value);
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading database: " << e.what() << std::endl;
        return false;
    }
}

// 🛠️ Save data to file
bool Database::save() {
    std::lock_guard<std::mutex> lock(dataMutex);
    json j(data);

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error saving database to " << filename << std::endl;
        return false;
    }

    outFile << j.dump(4);
    return true;
}

// 🛠️ Insert key-value pair and update B-Trees
void Database::insert(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(dataMutex);
    data[key] = value;
    keyIndex.insert(key);
    valueIndex.insert(value);
    updateIndices(key, value);
}

// 🛠️ Get value by key
std::string Database::get(const std::string& key) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    if (data.find(key) != data.end()) {
        return data.at(key);
    }
    return "";
}

// 🛠️ Remove key-value pair and update B-Trees
bool Database::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(dataMutex);
    if (data.erase(key) > 0) {
        keyIndex.remove(key);
        removeFromIndices(key);
        return true;
    }
    return false;
}

// 🛠️ Build B-Tree Indices
void Database::buildBTreeIndices() {
    for (const auto& [key, value] : data) {
        keyIndex.insert(key);
        valueIndex.insert(value);
    }
}

// 🛠️ Query by Prefix Using B-Tree
std::vector<std::string> Database::queryByPrefixBTree(const std::string& prefix) const {
    return keyIndex.rangeSearch(prefix);
}

// 🛠️ Query by Value Using B-Tree
std::vector<std::string> Database::queryByValueBTree(const std::string& value) const {
    return valueIndex.rangeSearch(value);
}

// 🛠️ Get all data
std::unordered_map<std::string, std::string> Database::getAllData() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return data;
}

// 🛠️ Update indices
void Database::updateIndices(const std::string& key, const std::string& value) {
    for (auto& [indexName, entries] : indices) {
        bool found = false;
        for (auto& entry : entries) {
            if (entry.key == key) {
                entry.value = value;
                found = true;
                break;
            }
        }
        if (!found) {
            entries.push_back({key, value});
        }
    }
}

// 🛠️ Remove from indices
void Database::removeFromIndices(const std::string& key) {
    for (auto& [_, entries] : indices) {
        entries.erase(std::remove_if(entries.begin(), entries.end(),
                    [&key](const IndexEntry& entry) { return entry.key == key; }),
                    entries.end());
    }
}

// 🛠️ Reset database
void Database::reset(const std::string& newFilename) {
    std::lock_guard<std::mutex> lock(dataMutex);
    filename = newFilename;
    data.clear();
    keyIndex = BTree<std::string>(3);
    valueIndex = BTree<std::string>(3);
    load();
}

// 🛠️ Print B-Tree indices
void Database::printKeyIndex() const {
    std::cout << "Key B-Tree Index: ";
    keyIndex.traverse();
    std::cout << std::endl;
}

void Database::printValueIndex() const {
    std::cout << "Value B-Tree Index: ";
    valueIndex.traverse();
    std::cout << std::endl;
}

// 🛠️ Query by exact value
std::vector<std::string> Database::queryByValue(const std::string& value) const {
    std::vector<std::string> results;
    std::lock_guard<std::mutex> lock(dataMutex);  // Ensure thread safety
    for (const auto& [key, val] : data) {
        if (val == value) results.push_back(key);
    }
    return results;
}

// 🛠️ Query by prefix
std::vector<std::string> Database::queryByPrefix(const std::string& prefix) const {
    std::vector<std::string> results;
    std::lock_guard<std::mutex> lock(dataMutex);  // Ensure thread safety
    for (const auto& [key, value] : data) {
        if (key.find(prefix) == 0) results.push_back(key);
    }
    return results;
}


// 🛠️ Export data to file
bool Database::exportTo(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    json j(data);
    std::ofstream outFile(filename);
    if (!outFile.is_open()) return false;
    outFile << j.dump(4);
    return true;
}

// 🛠️ Import data from file
bool Database::importFrom(const std::string& filename, bool merge) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) return false;

    json j;
    inFile >> j;

    std::lock_guard<std::mutex> lock(dataMutex);
    if (!merge) {
        data.clear();
    }

    for (auto& [key, value] : j.items()) {
        data[key] = value.is_string() ? value.get<std::string>() : value.dump();
        updateIndices(key, data[key]);
    }

    buildBTreeIndices();  // Rebuild B-Tree indices after import
    return true;
}

// 🛠️ Get value distribution
std::unordered_map<std::string, size_t> Database::getValueDistribution() const {
    std::unordered_map<std::string, size_t> distribution;
    for (const auto& [_, value] : data) {
        ++distribution[value];
    }
    return distribution;
}

// 🛠️ Get database size
size_t Database::size() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return data.size();
}
