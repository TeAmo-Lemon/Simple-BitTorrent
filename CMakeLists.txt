cmake_minimum_required(VERSION 3.00)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-DNDEBUG")

project(BitTorrent
        VERSION 0.1
        DESCRIPTION "simple torrent"
        LANGUAGES C CXX
        )

add_subdirectory(source)
# add_subdirectory(test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/include)
# add_subdirectory(googletest)



