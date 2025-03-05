#ifndef VERSION_CONTROL_H
#define VERSION_CONTROL_H

#include "/Users/gaganphadke/Versioning/versioned-db/include/Database.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <set>

// 🛠️ Commit structure
struct Commit {
    int id;
    std::string message;
    std::string branchName;
    std::string author;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> snapshot;
    int parentId;
    std::set<int> mergedFrom;
    std::unordered_map<std::string, std::set<Edge>> graphSnapshot;
};

// 🛠️ Commit log structure
struct CommitLog {
    int version;
    std::string author;
    std::string message;
    std::string date;
};

// 🛠️ VersionControl class
class VersionControl {
private:
    Database& db;
    int currentVersion;
    std::string currentBranch;
    std::string author;
    std::vector<std::shared_ptr<Commit>> commits;
    std::unordered_map<std::string, std::vector<int>> branches;
    std::unordered_map<std::string, std::vector<std::string>> staging;
    std::unordered_map<std::string, std::string> conflictMap;  // 🛠️ To track conflicts

    
    // 🛠️ Private functions for commit persistence
    void saveCommits() const;          // Save commits to file
    void loadCommits();                // Load commits from file

    bool saveCommitToFile(const std::shared_ptr<Commit>& commit);
    std::shared_ptr<Commit> loadCommitFromFile(int version);
    std::unordered_map<std::string, std::string> detectConflicts(
        const std::unordered_map<std::string, std::string>& base,
        const std::unordered_map<std::string, std::string>& theirs);

public:
    VersionControl(Database& db, const std::string& author = "user");
    
    // 🛠️ Basic operations
    bool commit(const std::string& message);
    bool checkout(int version);
    void log(int limit = -1) const;
    bool rollback(int version);

    std::vector<CommitLog> getCommitLogs(int limit = -1) const;
    
    // 🛠️ Branch management
    bool createBranch(const std::string& branchName);
    bool switchBranch(const std::string& branchName);
    bool merge(const std::string& branchName);
    std::vector<std::string> listBranches() const;
    
    // 🛠️ Staging area functions
    void stageChange(const std::string& key, const std::string& value);
    void unstageChange(const std::string& key);
    void listStagedChanges() const;
    
    // 🛠️ Advanced features
    bool tag(const std::string& tagName, int version = -1);
    bool checkoutTag(const std::string& tagName);
    std::vector<int> searchCommits(const std::string& searchTerm) const;
    bool rebase(const std::string& branchName);
    bool cherryPick(int commitId);
    
    // 🛠️ Conflict resolution
    bool resolveConflict(const std::string& key, const std::string& value);
    void listConflicts() const;
    
    // 🛠️ User settings
    void setAuthor(const std::string& name);
    std::string getAuthor() const;
    void getBranchHistory() const;
};

#endif
