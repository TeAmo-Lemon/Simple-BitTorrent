#include "gtest/gtest.h"
#include "parsing/torrent.h"
#include <fstream> // For file operations if needed for setup

// Test fixture for torrent tests, might be useful if we need to create a dummy torrent file
class TorrentTest : public ::testing::Test {
protected:
    // We can use a known small torrent file from the sample directory
    const std::string test_torrent_path = "../sample/puppy.torrent"; // Relative to build/test directory
};

TEST_F(TorrentTest, ConstructorAndBasicInfo) {
    ASSERT_NO_THROW({
        torrent t(test_torrent_path);
        // Basic checks, assuming puppy.torrent is a valid file and parsable
        ASSERT_FALSE(std::string(t.info_hash.begin(), t.info_hash.end()).empty());
        ASSERT_GT(t.length, 0);
        ASSERT_FALSE(t.name.empty());
        ASSERT_GT(t.piece_length, 0);
        ASSERT_GT(t.pieces, 0);
        ASSERT_FALSE(t.url.host.empty()); // Check if URL host is parsed (implies protocol was likely parsed)
    });
}

TEST_F(TorrentTest, GetPieceLength) {
    torrent t(test_torrent_path);
    // Assuming the torrent file has at least one piece
    ASSERT_GT(t.pieces, 0);
    // The piece_length for a regular piece should be t.piece_length
    if (t.pieces > 1) { // If there's more than one piece, the first piece is a full piece
        ASSERT_EQ(t.get_piece_length(0), t.piece_length);
    }
    // The last piece might be shorter
    unsigned int last_piece_index = t.pieces - 1;
    unsigned int last_piece_actual_length = t.length % t.piece_length;
    if (last_piece_actual_length == 0 && t.length > 0) { // If length is a multiple of piece_length
        last_piece_actual_length = t.piece_length;
    }
    if (t.length > 0) { // Avoid division by zero or issues with empty torrents
        ASSERT_EQ(t.get_piece_length(last_piece_index), last_piece_actual_length);
    }
}

TEST_F(TorrentTest, GetNBlocks) {
    torrent t(test_torrent_path);
    ASSERT_GT(t.pieces, 0);
    unsigned int expected_blocks_first_piece = (t.get_piece_length(0) + torrent::BLOCK_SIZE - 1) / torrent::BLOCK_SIZE;
    ASSERT_EQ(t.get_n_blocks(0), expected_blocks_first_piece);

    if (t.pieces > 1) {
        unsigned int last_piece_index = t.pieces - 1;
        unsigned int expected_blocks_last_piece = (t.get_piece_length(last_piece_index) + torrent::BLOCK_SIZE - 1) / torrent::BLOCK_SIZE;
        ASSERT_EQ(t.get_n_blocks(last_piece_index), expected_blocks_last_piece);
    }
}

TEST_F(TorrentTest, GetBlockLength) {
    torrent t(test_torrent_path);
    ASSERT_GT(t.pieces, 0);

    // Test first block of first piece
    unsigned int n_blocks_first_piece = t.get_n_blocks(0);
    ASSERT_GT(n_blocks_first_piece, 0);
    if (n_blocks_first_piece == 1) {
        ASSERT_EQ(t.get_block_length(0, 0), t.get_piece_length(0));
    } else {
        ASSERT_EQ(t.get_block_length(0, 0), torrent::BLOCK_SIZE);
    }

    // Test last block of first piece
    if (n_blocks_first_piece > 0) {
        unsigned int expected_len_last_block_first_piece = t.get_piece_length(0) - (n_blocks_first_piece - 1) * torrent::BLOCK_SIZE;
        ASSERT_EQ(t.get_block_length(0, n_blocks_first_piece - 1), expected_len_last_block_first_piece);
    }
    
    // If there's a last piece and it's different from the first
    if (t.pieces > 1) {
        unsigned int last_piece_idx = t.pieces -1;
        unsigned int n_blocks_last_piece = t.get_n_blocks(last_piece_idx);
        ASSERT_GT(n_blocks_last_piece, 0);
        if (n_blocks_last_piece == 1) {
            ASSERT_EQ(t.get_block_length(last_piece_idx, 0), t.get_piece_length(last_piece_idx));
        } else {
            ASSERT_EQ(t.get_block_length(last_piece_idx, 0), torrent::BLOCK_SIZE);
        }
        if (n_blocks_last_piece > 0) {
             unsigned int expected_len_last_block_last_piece = t.get_piece_length(last_piece_idx) - (n_blocks_last_piece - 1) * torrent::BLOCK_SIZE;
             ASSERT_EQ(t.get_block_length(last_piece_idx, n_blocks_last_piece - 1), expected_len_last_block_last_piece);
        }
    }
}

// Test with a non-existent torrent file
TEST_F(TorrentTest, NonExistentFile) {
    ASSERT_THROW(torrent t("../sample/non_existent_file.torrent"), std::runtime_error);
}

// It would be good to also test with a malformed torrent file
// For now, we'll assume the provided torrent files are well-formed. 