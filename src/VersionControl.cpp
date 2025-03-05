#include "/Users/gaganphadke/Versioning/versioned-db/include/VersionControl.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <ctime>  // For time formatting
#include </opt/homebrew/Cellar/nlohmann-json/3.11.3/include/nlohmann/json.hpp>
using json = nlohmann::json;

namespace fs = std::filesystem;

// 🛠️ Constructor
VersionControl::VersionControl(Database& db, const std::string& author)
    : db(db), currentVersion(0), currentBranch("main"), author(author) {
    if (!fs::exists("data")) fs::create_directory("data");
    branches["main"] = {};
    loadCommits();  // 🛠️ Load commits on start
}

// 🛠️ Save commits to file
void VersionControl::saveCommits() const {
    json j;
    for (const auto& commit : commits) {
        j.push_back({
            {"id", commit->id},
            {"message", commit->message},
            {"branchName", commit->branchName},
            {"author", commit->author},
            {"timestamp", std::chrono::system_clock::to_time_t(commit->timestamp)},
            {"snapshot", commit->snapshot}
        });
    }
    std::ofstream outFile("data/commits.json");
    if (outFile.is_open()) {
        outFile << j.dump(4);
        outFile.close();
    } else {
        std::cerr << "Failed to open commits.json for writing!" << std::endl;
    }
}

// 🛠️ Load commits from file
void VersionControl::loadCommits() {
    std::ifstream inFile("data/commits.json");
    if (!inFile.is_open()) {
        std::cerr << "[Debug] commits.json not found. Starting with an empty commit history." << std::endl;
        return;
    }

    // Check if the file is empty
    inFile.seekg(0, std::ios::end);
    if (inFile.tellg() == 0) {
        std::cerr << "[Debug] commits.json is empty. Starting with an empty commit history." << std::endl;
        inFile.close();
        return;
    }
    inFile.seekg(0, std::ios::beg);

    try {
        json j;
        inFile >> j;
        for (const auto& item : j) {
            auto commit = std::make_shared<Commit>();
            commit->id = item["id"];
            commit->message = item["message"];
            commit->branchName = item["branchName"];
            commit->author = item["author"];
            commit->timestamp = std::chrono::system_clock::from_time_t(item["timestamp"]);
            commit->snapshot = item["snapshot"].get<std::unordered_map<std::string, std::string>>();
            commits.push_back(commit);
            branches[commit->branchName].push_back(commit->id);
            currentVersion = std::max(currentVersion, commit->id + 1);
        }
        std::cout << "[Debug] Loaded " << commits.size() << " commits from commits.json." << std::endl;
    } catch (const json::parse_error& e) {
        std::cerr << "Error loading commits.json: " << e.what() << std::endl;
    }
}


// 🛠️ Commit changes
bool VersionControl::commit(const std::string& message) {
    auto commit = std::make_shared<Commit>();
    commit->id = currentVersion++;
    commit->message = message;
    commit->branchName = currentBranch;
    commit->author = author;
    commit->timestamp = std::chrono::system_clock::now();
    commit->snapshot = db.getAllData();
    commit->graphSnapshot = graph; 
    commits.push_back(commit);
    branches[currentBranch].push_back(commit->id);
    saveCommits();  // 🛠️ Save commits to file after every commit
    return true;
}

// 🛠️ Get detailed commit logs
std::vector<CommitLog> VersionControl::getCommitLogs(int limit) const {
    std::vector<CommitLog> logs;
    int count = 0;
    for (auto it = commits.rbegin(); it != commits.rend(); ++it) {
        if (limit != -1 && count >= limit) break;

        std::time_t timestamp = std::chrono::system_clock::to_time_t((*it)->timestamp);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));

        logs.push_back({
            (*it)->id,
            (*it)->author,
            (*it)->message,
            std::string(buffer)
        });

        ++count;
    }
    return logs;
}

// 🛠️ Print commit history
void VersionControl::log(int limit) const {
    auto logs = getCommitLogs(limit);
    if (logs.empty()) {
        std::cout << "No commit history available." << std::endl;
        return;
    }

    std::cout << "===== Commit History =====" << std::endl;
    for (const auto& log : logs) {
        std::cout << "Version: " << log.version 
                  << " | Author: " << log.author 
                  << " | Date: " << log.date << std::endl
                  << "Message: " << log.message << std::endl
                  << "--------------------------" << std::endl;
    }
}

// 🛠️ Merge branches
bool VersionControl::merge(const std::string& branchName) {
    if (branches.find(branchName) == branches.end()) {
        std::cout << "Branch not found: " << branchName << std::endl;
        return false;
    }

    // Track conflicts
    std::unordered_map<std::string, std::string> conflicts;

    // Iterate through commits in the branch to merge
    for (int version : branches[branchName]) {
        if (version >= commits.size()) return false;

        // Compare snapshots for conflicts
        for (const auto& [key, value] : commits[version]->snapshot) {
            std::string currentValue = db.get(key);

            if (!currentValue.empty() && currentValue != value) {
                // 🛠️ Conflict detected
                conflicts[key] = value;
            } else {
                // No conflict, apply change
                db.insert(key, value);
            }
        }
    }

    if (!conflicts.empty()) {
        // Save conflicts for later resolution
        conflictMap = conflicts;
        std::cout << "Merge completed with conflicts. Use 'conflicts' command to list them.\n";
    } else {
        std::cout << "Merge completed successfully with no conflicts.\n";
    }

    return true;
}


// 🛠️ Resolve conflicts
// 🛠️ Resolve conflicts manually
bool VersionControl::resolveConflict(const std::string& key, const std::string& value) {
    if (conflictMap.find(key) != conflictMap.end()) {
        db.insert(key, value);  // Apply user-provided value
        conflictMap.erase(key);  // Remove resolved conflict
        std::cout << "Conflict resolved for key: " << key << "\n";
        db.save();  // Save the database after resolving conflict
        return true;
    } else {
        std::cout << "No conflict found for key: " << key << "\n";
        return false;
    }
}


// 🛠️ Checkout a specific version
bool VersionControl::checkout(int version) {
    if (version < 0 || version >= commits.size()) return false;
    db.reset("data/mydb.json");
    for (const auto& [key, value] : commits[version]->snapshot) {
        db.insert(key, value);
    }
    graph = commits[version]->graphSnapshot;  
    return db.save();
}

// 🛠️ Tag a version
bool VersionControl::tag(const std::string& tagName, int version) {
    if (version < 0 || version >= commits.size()) return false;
    std::ofstream outFile("data/tag_" + tagName + ".txt");
    if (!outFile.is_open()) return false;
    outFile << version;
    return true;
}

// 🛠️ Checkout a tag
bool VersionControl::checkoutTag(const std::string& tagName) {
    std::ifstream inFile("data/tag_" + tagName + ".txt");
    if (!inFile.is_open()) return false;
    int version;
    inFile >> version;
    return checkout(version);
}

// 🛠️ Search commits by keyword
std::vector<int> VersionControl::searchCommits(const std::string& searchTerm) const {
    std::vector<int> results;
    for (const auto& commit : commits) {
        if (commit->message.find(searchTerm) != std::string::npos) {
            results.push_back(commit->id);
        }
    }
    return results;
}

// 🛠️ Stage a change
void VersionControl::stageChange(const std::string& key, const std::string& value) {
    staging[currentBranch].push_back(key);
    db.insert(key, value);
}

// 🛠️ Unstage a change
void VersionControl::unstageChange(const std::string& key) {
    auto& staged = staging[currentBranch];
    staged.erase(std::remove(staged.begin(), staged.end(), key), staged.end());
}

// 🛠️ List staged changes
void VersionControl::listStagedChanges() const {
    if (staging.count(currentBranch) == 0) {
        std::cout << "No staged changes." << std::endl;
        return;
    }
    for (const auto& key : staging.at(currentBranch)) {
        std::cout << "Staged: " << key << std::endl;
    }
}

// 🛠️ List conflicts (dummy implementation for now)
// 🛠️ List conflicts if any exist
void VersionControl::listConflicts() const {
    if (conflictMap.empty()) {
        std::cout << "No conflicts detected.\n";
    } else {
        std::cout << "Conflicts detected:\n";
        for (const auto& [key, conflictingValue] : conflictMap) {
            std::cout << "Key: " << key << " | Conflicting Value: " << conflictingValue << "\n";
        }
    }
}


// 🛠️ Get branch history (dummy implementation for now)
void VersionControl::getBranchHistory() const {
    std::cout << "Branch history is not implemented yet." << std::endl;
}

bool VersionControl::createBranch(const std::string& branchName) {
    if (branches.find(branchName) != branches.end()) return false;
    branches[branchName] = branches[currentBranch];
    return true;
}

// 🛠️ Switch to a different branch
bool VersionControl::switchBranch(const std::string& branchName) {
    if (branches.find(branchName) == branches.end()) return false;
    currentBranch = branchName;
    return true;
}

// 🛠️ Rollback to a specific version
bool VersionControl::rollback(int version) {
    return checkout(version);
}

// 🛠️ Get author
std::string VersionControl::getAuthor() const {
    return author;
}

// 🛠️ Set author
void VersionControl::setAuthor(const std::string& name) {
    author = name;
}

// 🛠️ List branches
std::vector<std::string> VersionControl::listBranches() const {
    std::vector<std::string> branchList;
    for (const auto& [branch, _] : branches) branchList.push_back(branch);
    return branchList;
}