unsigned int torrent::get_piece_length(unsigned int piece)
{
    // 检查索引有效性
    if (piece >= pieces)
    {
        throw runtime_error("Invalid piece index");
    }

    // 对于最后一个片段，长度可能小于标准片段长度
    if (piece == pieces - 1)
    {
        unsigned int last_piece_length = length % piece_length;
        return last_piece_length == 0 ? piece_length : last_piece_length;
    }

    return piece_length;
}

unsigned int torrent::get_n_blocks(unsigned int piece)
{
    unsigned int piece_len = get_piece_length(piece);
    return (piece_len + BLOCK_SIZE - 1) / BLOCK_SIZE; // 向上取整
}

unsigned int torrent::get_block_length(unsigned int piece, unsigned int block_index)
{
    unsigned int piece_len = get_piece_length(piece);
    unsigned int n_blocks = get_n_blocks(piece);

    // 检查块索引有效性
    if (block_index >= n_blocks)
    {
        throw runtime_error("Invalid block index");
    }

    // 对于最后一个块，长度可能小于标准块大小
    if (block_index == n_blocks - 1)
    {
        unsigned int last_block_length = piece_len % BLOCK_SIZE;
        return last_block_length == 0 ? BLOCK_SIZE : last_block_length;
    }

    return BLOCK_SIZE;
}