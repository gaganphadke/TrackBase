#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>
#include<set>
#include "/Users/gaganphadke/Versioning/versioned-db/include/BTree.h"  // Include B-Tree header

// Data type enumeration for flexible storage
enum DataType {
    STRING,
    INTEGER,
    FLOAT,
    BOOLEAN,
    ARRAY,
    OBJECT
};

// Structure to hold index entries
struct IndexEntry {
    std::string key;
    std::string value;
};

struct Edge {
    std::string to;
    int weight;

    // Comparator for set
    bool operator<(const Edge& other) const {
        return to < other.to;
    }
};
extern std::unordered_map<std::string, std::set<Edge>> graph;

// Database class with B-Tree indexing
class Database {
private:
    std::string filename;
    std::unordered_map<std::string, std::string> data;
    std::unordered_map<std::string, std::vector<IndexEntry>> indices;
    mutable std::mutex dataMutex;

    BTree<std::string> keyIndex;    // B-Tree for key indexing
    BTree<std::string> valueIndex;  // B-Tree for value indexing

    bool indexExists(const std::string& indexName) const;
    void updateIndices(const std::string& key, const std::string& value);
    void removeFromIndices(const std::string& key);
    void buildBTreeIndices();  // Builds B-Tree indices

public:
    // Constructor
    Database(const std::string& filename);

    // Basic operations
    bool load();
    bool save();
    void insert(const std::string& key, const std::string& value);
    std::string get(const std::string& key) const;
    bool remove(const std::string& key);
    std::unordered_map<std::string, std::string> getAllData() const;

    void insertNode(const std::string& node);                   
    void insertEdge(const std::string& from, const std::string& to, int weight);  
    std::vector<std::string> bfs(const std::string& start);      
    std::vector<std::string> dfs(const std::string& start);

    // Advanced querying
    std::vector<std::string> query(
        const std::function<bool(const std::string&, const std::string&)>& predicate) const;
    std::vector<std::string> queryByPrefix(const std::string& prefix) const;
    std::vector<std::string> queryByValue(const std::string& value) const;
    std::vector<std::string> queryByValuePattern(const std::string& pattern) const;

    // B-Tree-based querying
    std::vector<std::string> queryByPrefixBTree(const std::string& prefix) const;
    std::vector<std::string> queryByValueBTree(const std::string& value) const;

    // Indexing
    bool createIndex(const std::string& indexName, 
                     const std::function<std::string(const std::string&)>& indexer);
    std::vector<std::string> queryByIndex(const std::string& indexName, const std::string& value) const;

    // Batch operations
    bool batchInsert(const std::unordered_map<std::string, std::string>& entries);
    bool batchRemove(const std::vector<std::string>& keys);

    // Statistics
    size_t size() const;
    std::unordered_map<std::string, size_t> getValueDistribution() const;

    // Caching and performance
    void optimize();
    void clearCache();

    // Import/Export
    bool importFrom(const std::string& filename, bool merge = false);
    bool exportTo(const std::string& filename) const;

    // Reset database
    void reset(const std::string& newFilename);

    // Print B-Tree indices (for debugging)
    void printKeyIndex() const;
    void printValueIndex() const;
};

#endif
