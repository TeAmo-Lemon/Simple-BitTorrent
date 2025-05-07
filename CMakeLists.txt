cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-DNDEBUG")

project(BitTorrent
        VERSION 0.1
        DESCRIPTION "simple torrent"
        LANGUAGES C CXX
        )

# Enable testing
enable_testing()

# Find GTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(test)

# 将源文件（除了 main.cpp）定义为一个列表
set(BITTORRENT_SOURCES
    parsing/bencode.cpp
    parsing/buffer.cpp
    parsing/torrent.cpp
    tracker/http.cpp
    tracker/https.cpp
    tracker/ssl_transport.cpp
    tracker/tracker.cpp
    tracker/transport.cpp
    tracker/url.cpp
    download/connection.cpp
    download/download.cpp
    download/farm.cpp
    download/message.cpp
    download/peer_id.cpp
    download/worker.cpp
)

# 创建库 BitTorrent_lib
add_library(BitTorrent_lib ${BITTORRENT_SOURCES})

# 为库设置包含目录
target_include_directories(BitTorrent_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 创建主可执行文件
add_executable(BitTorrentApp main.cpp)

# 链接主可执行文件到库和其他依赖
target_link_libraries(BitTorrentApp PRIVATE BitTorrent_lib ssl crypto)




