# Set up testing
set(TEST_SOURCES
    test_bencode.cpp
    test_torrent.cpp
)

# Create test executable
add_executable(bittorrent_tests ${TEST_SOURCES})

# Link against GTest and your project libraries
target_link_libraries(bittorrent_tests
    GTest::GTest
    GTest::Main
    BitTorrent_lib
    ssl
    crypto
)

# 在 target_link_libraries 之前添加
target_include_directories(bittorrent_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Register tests with output
gtest_discover_tests(bittorrent_tests
    PROPERTIES
    ENVIRONMENT "GTEST_OUTPUT=xml:${CMAKE_BINARY_DIR}/test_detail.xml"
)

# Enable test output
set_tests_properties(${ALL_TESTS} PROPERTIES
    ENVIRONMENT "CTEST_OUTPUT_ON_FAILURE=1"
)