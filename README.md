# Versioned Database System  

## Overview  
The **Versioned Database System** is a high-performance, C++-based key-value and graph-structured database with built-in **version control** and **branching** capabilities. It supports **data versioning**, **commit history tracking**, **branching**, **merging**, and **conflict resolution**, similar to Git, but designed specifically for structured data.  

The project also includes a **Flask web interface** for user interaction and a **client-server architecture** using **C++ sockets** to manage remote commit operations.  


## Features  

### **Database Functionality**  
- **Key-Value Operations:** Insert, retrieve, delete, query by prefix or value.  
- **Graph-Based Storage:** Nodes and weighted edges support BFS, DFS, and shortest path queries.  
- **Export & Import:** Save and load database states from JSON files.  
- **Statistics:** Retrieve key-value and graph data insights.  

### **Version Control**  
- **Staging & Committing:** Track changes and commit snapshots.  
- **Checkout & Rollback:** Revert to previous database versions.  
- **Branching & Merging:** Create branches and merge changes.  
- **Conflict Detection & Resolution:** Identify and resolve data conflicts.  

### **Networking & Remote Operations**  
- **Client-Server Model:** Communicate over TCP sockets.  
- **Commit Logging & Querying:** View commit history across versions.  
- **Flask Web Interface:** Manage database operations via a browser UI.  


## Technologies Used  

- **C++** → Backend logic, file handling, and graph processing.  
- **Flask (Python)** → Web-based UI for database interaction.  
- **nlohmann/json** → JSON-based data storage and versioning.  
- **Sockets (TCP/IP)** → Client-server communication.  
- **CMake** → Cross-platform build system.  


## Installation & Setup  

### **1. Clone the Repository**  
sh
git clone https://github.com/your-repo/versioned-db.git
cd versioned-db


### **2. Install Dependencies**  
Ensure you have CMake, g++ (or Clang), and Python installed.  
For JSON parsing, install nlohmann/json:  
sh
brew install nlohmann-json  # macOS  
sudo apt install nlohmann-json-dev  # Ubuntu  

### **3. Build & Run the C++ Server**  
sh
mkdir build && cd build
cmake ..
make
./VersionedDB


### **4. Start the Flask Web Interface**  
sh
cd web_interface/
pip install -r requirements.txt
python app.py

Access the UI at http://localhost:5000.  


## Usage  

### **Command-Line Interface (CLI)**  
Run ./VersionedDB, then use the following commands:  

#### **Database Operations**  
sh
insert key1 value1        # Insert a key-value pair  
get key1                  # Retrieve value by key  
query prefix_             # Find keys with a specific prefix  
export data.json          # Save database to a file  


#### **Graph Operations**  
sh
addnode A                 # Add a node to the graph  
addedge A B 10            # Connect A to B with weight 10  
bfs A                     # Perform Breadth-First Search from A  
dfs A                     # Perform Depth-First Search from A  
shortestpath A B          # Find the shortest path from A to B  


#### **Version Control**  
sh
stage key1 value1         # Stage a change  
commit "Initial commit"   # Save changes with a message  
log                       # View commit history  
checkout 2                # Revert to version 2  
rollback 1                # Undo to version 1  


#### **Branching & Merging**  
sh
branch feature-1          # Create a new branch  
switch feature-1          # Switch to the branch  
merge main                # Merge changes from main  
conflicts                 # View merge conflicts  
resolve key1 new_value    # Manually resolve conflict  


#### **Networking & Remote Commits**  
sh
client commit "New changes"  # Commit remotely to server  


---

## Roadmap & Future Enhancements  

- **Graph Database Enhancements:** Extend querying for **connected components, cycle detection, and network analysis**.  
- **AI-Driven Conflict Resolution:** Use **machine learning** to suggest the best conflict resolutions.  
- **Full REST API:** Convert CLI commands into a **RESTful API** for seamless integration.  
- **Distributed Database Support:** Expand version control to **multiple servers** for scalability.  

---

## Contributors  
- **Gagan Phadke** - [GitHub](https://github.com/gaganphadke)  🚀
