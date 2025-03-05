#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "/Users/gaganphadke/Versioning/versioned-db/include/Database.h"
#include "/Users/gaganphadke/Versioning/versioned-db/include/VersionControl.h"
#include "Client.cpp" 

// Helper function to split string by spaces while preserving quoted sections
std::vector<std::string> parseCommand(const std::string& commandLine) {
    std::vector<std::string> args;
    std::string current;
    bool inQuotes = false;
    
    for (char c : commandLine) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        args.push_back(current);
    }
    
    return args;
}

void printMenu() {
    std::cout << "\n====== Versioned Database System ======\n";
    std::cout << "Database Commands:\n";
    std::cout << "  insert <key> <value>           - Insert or update a key-value pair\n";
    std::cout << "  get <key>                      - Retrieve value by key\n";
    std::cout << "  remove <key>                   - Remove a key-value pair\n";
    std::cout << "  query <prefix>                 - Find all keys with prefix\n";
    std::cout << "  queryvalue <value>             - Find keys with specific value\n";
    std::cout << "  export <filename>              - Export database to file\n";
    std::cout << "  import <filename> [--merge]    - Import database from file\n";
    std::cout << "  stats                          - Show database statistics\n";

    std::cout << "\nGraph Commands:\n";
    std::cout << "  addnode <node>                 - Add a node to the graph\n";
    std::cout << "  addedge <from> <to> <weight>   - Add an edge with weight between nodes\n";
    std::cout << "  bfs <start>                    - Perform BFS traversal\n";
    std::cout << "  dfs <start>                    - Perform DFS traversal\n";
    
    std::cout << "\nVersion Control Commands:\n";
    std::cout << "  stage <key> <value>            - Stage a change for commit\n";
    std::cout << "  unstage <key>                  - Remove a change from staging\n";
    std::cout << "  status                         - Show staged changes\n";
    std::cout << "  commit <message>               - Commit staged changes\n";
    std::cout << "  checkout <version>             - Checkout a specific version\n";
    std::cout << "  log [limit]                    - Show commit history\n";
    std::cout << "  rollback <version>             - Roll back to a specific version\n";
    
    std::cout << "\nBranch Commands:\n";
    std::cout << "  branch <name>                  - Create a new branch\n";
    std::cout << "  branches                       - List all branches\n";
    std::cout << "  switch <branch>                - Switch to a different branch\n";
    std::cout << "  merge <branch>                 - Merge another branch into current\n";
    std::cout << "  history                        - Show current branch history\n";
    
    std::cout << "\nAdvanced Commands:\n";
    std::cout << "  tag <name> [version]           - Create a named tag for a version\n";
    std::cout << "  checkouttag <name>             - Checkout a version by tag\n";
    std::cout << "  search <term>                  - Search commits for term\n";
    std::cout << "  conflicts                      - List merge conflicts\n";
    std::cout << "  resolve <key> <value>          - Resolve a merge conflict\n";
    std::cout << "  author <name>                  - Set author name\n";
    
    std::cout << "\nOther Commands:\n";
    std::cout << "  help                           - Show this menu\n";
    std::cout << "  exit                           - Exit the application\n";
    std::cout << "======================================\n";
}

void printError(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
}

int main(int argc, char* argv[]) {
    // Database filename from command line or default
    std::string dbFilename = "data/mydb.json";
    std::string authorName = "user";
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--db" && i + 1 < argc) {
            dbFilename = argv[++i];
        } else if (arg == "--author" && i + 1 < argc) {
            authorName = argv[++i];
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--db FILENAME] [--author NAME]" << std::endl;
            return 0;
        }
    }
    
    // Initialize database and version control
    Database db(dbFilename);
    if (!db.load()) {
        std::cout << "Creating new database at " << dbFilename << std::endl;
    } else {
        std::cout << "Loaded database with " << db.size() << " entries" << std::endl;
    }
    
    VersionControl vc(db, authorName);
    std::cout << "Version control initialized with author: " << vc.getAuthor() << std::endl;
    
    // Main command loop
    printMenu();
    std::string commandLine;
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, commandLine);
        
        if (commandLine.empty()) continue;
        
        try {
            auto args = parseCommand(commandLine);
            if (args.empty()) continue;
            
            std::string command = args[0];
            
            // Convert command to lowercase
            std::transform(command.begin(), command.end(), command.begin(), ::tolower);
            
            // Database commands
            if (command == "insert" && args.size() >= 3) {
                std::string key = args[1];
                std::string value = args[2];
                
                // Combine remaining args if there are more
                for (size_t i = 3; i < args.size(); i++) {
                    value += " " + args[i];
                }
                
                db.insert(key, value);
                std::cout << "Inserted: " << key << " = " << value << std::endl;
                db.save();
            }
            else if (command == "get" && args.size() >= 2) {
                std::string key = args[1];
                std::string value = db.get(key);
                
                if (value.empty()) {
                    std::cout << "Key not found: " << key << std::endl;
                } else {
                    std::cout << key << " = " << value << std::endl;
                }
            }
            else if (command == "remove" && args.size() >= 2) {
                std::string key = args[1];
                if (db.remove(key)) {
                    std::cout << "Removed key: " << key << std::endl;
                    db.save();
                } else {
                    std::cout << "Key not found: " << key << std::endl;
                }
            }
            else if (command == "query" && args.size() >= 2) {
                std::string prefix = args[1];
                auto results = db.queryByPrefix(prefix);
                
                std::cout << "Found " << results.size() << " keys with prefix '" << prefix << "':" << std::endl;
                for (const auto& key : results) {
                    std::cout << "  " << key << " = " << db.get(key) << std::endl;
                }
            }
            else if (command == "queryvalue" && args.size() >= 2) {
                std::string value = args[1];
                auto results = db.queryByValue(value);
                
                std::cout << "Found " << results.size() << " keys with value '" << value << "':" << std::endl;
                for (const auto& key : results) {
                    std::cout << "  " << key << std::endl;
                }
            }
            else if (command == "export" && args.size() >= 2) {
                std::string filename = args[1];
                if (db.exportTo(filename)) {
                    std::cout << "Database exported to " << filename << std::endl;
                } else {
                    printError("Failed to export database");
                }
            }
            else if (command == "import" && args.size() >= 2) {
                std::string filename = args[1];
                bool merge = (args.size() >= 3 && args[2] == "--merge");
                
                if (db.importFrom(filename, merge)) {
                    std::cout << "Database imported from " << filename 
                              << (merge ? " (merged)" : " (replaced)") << std::endl;
                    db.save();
                } else {
                    printError("Failed to import database");
                }
            }
            else if (command == "stats") {
                std::cout << "Database statistics:" << std::endl;
                std::cout << "  Total entries: " << db.size() << std::endl;
                
                auto distribution = db.getValueDistribution();
                if (!distribution.empty()) {
                    std::cout << "  Value distribution (top 5):" << std::endl;
                    
                    // Sort by frequency
                    std::vector<std::pair<std::string, size_t>> sorted;
                    for (const auto& [value, count] : distribution) {
                        sorted.push_back({value, count});
                    }
                    
                    std::sort(sorted.begin(), sorted.end(), 
                             [](const auto& a, const auto& b) { return a.second > b.second; });
                    
                    int shown = 0;
                    for (const auto& [value, count] : sorted) {
                        if (++shown > 5) break;
                        std::cout << "    " << value << ": " << count << std::endl;
                    }
                }
            }

            //Graph
            else if (command == "addnode" && args.size() >= 2) {
                db.insertNode(args[1]);
                std::cout << "Node " << args[1] << " added.\n";
            }
            else if (command == "addedge" && args.size() >= 4) {
                db.insertEdge(args[1], args[2], std::stoi(args[3]));
                std::cout << "Edge added between " << args[1] << " and " << args[2] << ".\n";
            }
            else if (command == "bfs" && args.size() >= 2) {
                auto result = db.bfs(args[1]);
                for (const auto& node : result) std::cout << node << " ";
                std::cout << "\n";
            }
            else if (command == "dfs" && args.size() >= 2) {
                auto result = db.dfs(args[1]);
                for (const auto& node : result) std::cout << node << " ";
                std::cout << "\n";
            }
            
            
            // Version control commands
            else if (command == "stage" && args.size() >= 3) {
                std::string key = args[1];
                std::string value = args[2];
                
                // Combine remaining args if there are more
                for (size_t i = 3; i < args.size(); i++) {
                    value += " " + args[i];
                }
                
                vc.stageChange(key, value);
            }
            else if (command == "unstage" && args.size() >= 2) {
                vc.unstageChange(args[1]);
            }
            else if (command == "status") {
                vc.listStagedChanges();
            }
            else if (command == "commit" && args.size() >= 2) {
                std::string message = commandLine.substr(7);  // Extract message after "commit "
                std::cout << "[Client] Sending commit to server...\n";
                sendCommit("commit " + message);
            }
            else if (command == "checkout" && args.size() >= 2) {
                try {
                    int version = std::stoi(args[1]);
                    if (!vc.checkout(version)) {
                        printError("Failed to checkout version " + args[1]);
                    }
                } catch (const std::exception& e) {
                    printError("Invalid version number: " + args[1]);
                }
            }
            else if (command == "log") {
                int limit = -1;
                if (args.size() >= 2) {
                    try {
                        limit = std::stoi(args[1]);
                    } catch (...) {
                        limit = -1;
                    }
                }
                auto logs = vc.getCommitLogs(limit);
                std::cout << "===== Commit History =====\n";
                for (const auto& log : logs) {
                    std::cout << "Version " << log.version << " | Author: " << log.author
                              << " | Message: " << log.message << " | Date: " << log.date << "\n";
                }
                std::cout << "==========================\n";
            }
            else if (command == "rollback" && args.size() >= 2) {
                try {
                    int version = std::stoi(args[1]);
                    if (!vc.rollback(version)) {
                        printError("Failed to rollback to version " + args[1]);
                    }
                } catch (const std::exception& e) {
                    printError("Invalid version number: " + args[1]);
                }
            }
            else if (command == "branch" && args.size() >= 2) {
                if (!vc.createBranch(args[1])) {
                    printError("Failed to create branch " + args[1]);
                }
            }
            else if (command == "branches") {
                auto branches = vc.listBranches();
                std::cout << "Branches:" << std::endl;
                for (const auto& branch : branches) {
                    std::cout << "  " << branch << std::endl;
                }
            }
            else if (command == "switch" && args.size() >= 2) {
                if (!vc.switchBranch(args[1])) {
                    printError("Failed to switch to branch " + args[1]);
                }
            }
            else if (command == "merge" && args.size() >= 2) {
                if (!vc.merge(args[1])) {
                    printError("Merge with conflicts or failed. Use 'conflicts' to view issues.");
                }
            }
            else if (command == "history") {
                vc.getBranchHistory();
            }
            else if (command == "tag" && args.size() >= 2) {
                int version = -1;
                if (args.size() >= 3) {
                    try {
                        version = std::stoi(args[2]);
                    } catch (...) {
                        printError("Invalid version number: " + args[2]);
                        continue;
                    }
                }
                
                if (!vc.tag(args[1], version)) {
                    printError("Failed to create tag " + args[1]);
                }
            }
            else if (command == "checkouttag" && args.size() >= 2) {
                if (!vc.checkoutTag(args[1])) {
                    printError("Failed to checkout tag " + args[1]);
                }
            }
            else if (command == "search" && args.size() >= 2) {
                std::string term = args[1];
                auto results = vc.searchCommits(term);
                
                std::cout << "Found " << results.size() << " commits matching '" << term << "':" << std::endl;
                for (int version : results) {
                    std::cout << "  Version " << version << std::endl;
                }
            }
            else if (command == "conflicts") {
                vc.listConflicts();
            }
            else if (command == "resolve" && args.size() >= 3) {
                std::string key = args[1];
                std::string value = args[2];
                vc.resolveConflict(key, value);  // 🛠️ Resolve conflict
            }
            else if (command == "author" && args.size() >= 2) {
                vc.setAuthor(args[1]);
            }
            else if (command == "help") {
                printMenu();
            }
            else if (command == "exit") {
                break;
            }
            else {
                printError("Unknown or incomplete command: " + commandLine);
                std::cout << "Type 'help' for a list of commands." << std::endl;
            }
        } catch (const std::exception& e) {
            printError(std::string("Exception: ") + e.what());
        }
    }
    
    std::cout << "Exiting. Database saved." << std::endl;
    return 0;
}