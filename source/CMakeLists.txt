set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES *.cpp)

set(SOURCES ${SOURCES})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(${BINARY} ${SOURCES})

target_link_libraries(${BINARY} -lcrypto -lpthread -lssl)

add_library(${BINARY}_lib STATIC ${SOURCES})

target_link_libraries(${BINARY}_lib -lcrypto -lpthread -lssl)