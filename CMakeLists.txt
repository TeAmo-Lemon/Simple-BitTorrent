cmake_minimum_required(VERSION 3.00)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-DNDEBUG")

project(BitTorrent
        VERSION 0.1
        DESCRIPTION "simple torrent"
        LANGUAGES C CXX
        )

# add_subdirectory(source)
# add_subdirectory(test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/include)
# add_subdirectory(googletest)

add_executable(BitTorrentApp
    main.cpp
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

target_link_libraries(BitTorrentApp PRIVATE ssl crypto)



