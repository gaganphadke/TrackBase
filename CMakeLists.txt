cmake_minimum_required(VERSION 3.10)
project(VersionedDB)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(/opt/homebrew/Cellar/nlohmann-json/3.11.3/include)

# 🛠️ Create main executable
add_executable(VersionedDB 
    src/main.cpp 
    src/Database.cpp 
    src/VersionControl.cpp
)

# 🛠️ Create server executable
add_executable(Server 
    src/Server.cpp 
    src/Database.cpp 
    src/VersionControl.cpp
)

# 🛠️ Create client executable

# 🛠️ Link pthread for multithreading support
target_link_libraries(VersionedDB pthread)
target_link_libraries(Server pthread)
