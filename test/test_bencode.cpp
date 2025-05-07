#include "gtest/gtest.h"
#include "parsing/bencode.h"
#include "parsing/buffer.h"
#include <string>
#include <vector>
#include <map>

// 测试解析 Bencode 编码的整数
TEST(BencodeTest, ParseInteger) {
    const char* s_buf = "i42e";
    buffer buf(s_buf, s_buf + strlen(s_buf));
    bencode::item result = bencode::parse(buf);
    ASSERT_EQ(result.t, bencode::type::i);
    ASSERT_EQ(std::any_cast<long long>(result.data), 42LL);
}

// 测试解析 Bencode 编码的字符串
TEST(BencodeTest, ParseString) {
    const char* s_buf = "4:spam";
    buffer buf(s_buf, s_buf + strlen(s_buf));
    bencode::item result = bencode::parse(buf);
    ASSERT_EQ(result.t, bencode::type::bs);
    ASSERT_EQ(std::string(std::any_cast<buffer>(result.data).begin(), std::any_cast<buffer>(result.data).end()), "spam");
}

// 测试解析 Bencode 编码的列表
TEST(BencodeTest, ParseList) {
    const char* s_buf = "li42e4:spame";
    buffer buf(s_buf, s_buf + strlen(s_buf));
    bencode::item result = bencode::parse(buf);
    ASSERT_EQ(result.t, bencode::type::l);
    auto list_items = std::any_cast<std::vector<bencode::item>>(result.data);
    ASSERT_EQ(list_items.size(), 2);
    ASSERT_EQ(list_items[0].t, bencode::type::i);
    ASSERT_EQ(std::any_cast<long long>(list_items[0].data), 42LL);
    ASSERT_EQ(list_items[1].t, bencode::type::bs);
    ASSERT_EQ(std::string(std::any_cast<buffer>(list_items[1].data).begin(), std::any_cast<buffer>(list_items[1].data).end()), "spam");
}

// 测试解析 Bencode 编码的字典
TEST(BencodeTest, ParseDictionary) {
    const char* s_buf = "d3:keyi42e4:spamli1ei2ee";
    buffer buf(s_buf, s_buf + strlen(s_buf));
    bencode::item result = bencode::parse(buf);
    ASSERT_EQ(result.t, bencode::type::d);
    auto dict_items = std::any_cast<std::map<bencode::item, bencode::item>>(result.data);
    ASSERT_EQ(dict_items.size(), 2);

    bencode::item key_item;
    key_item.t = bencode::type::bs;
    const char* s_key = "key";
    key_item.data = buffer(s_key, s_key + strlen(s_key));

    bencode::item spam_item;
    spam_item.t = bencode::type::bs;
    const char* s_spam = "spam";
    spam_item.data = buffer(s_spam, s_spam + strlen(s_spam));

    ASSERT_TRUE(dict_items.count(key_item));
    ASSERT_EQ(dict_items[key_item].t, bencode::type::i);
    ASSERT_EQ(std::any_cast<long long>(dict_items[key_item].data), 42LL);

    ASSERT_TRUE(dict_items.count(spam_item));
    ASSERT_EQ(dict_items[spam_item].t, bencode::type::l);
    auto inner_list = std::any_cast<std::vector<bencode::item>>(dict_items[spam_item].data);
    ASSERT_EQ(inner_list.size(), 2);
    ASSERT_EQ(inner_list[0].t, bencode::type::i);
    ASSERT_EQ(std::any_cast<long long>(inner_list[0].data), 1LL);
    ASSERT_EQ(inner_list[1].t, bencode::type::i);
    ASSERT_EQ(std::any_cast<long long>(inner_list[1].data), 2LL);
}

// 测试编码整数
TEST(BencodeTest, EncodeInteger) {
    bencode::item item_to_encode;
    item_to_encode.t = bencode::type::i;
    item_to_encode.data = 42LL;
    buffer encoded_buf = bencode::encode(item_to_encode);
    ASSERT_EQ(std::string(encoded_buf.begin(), encoded_buf.end()), "i42e");
}

// 测试编码字符串
TEST(BencodeTest, EncodeString) {
    bencode::item item_to_encode;
    item_to_encode.t = bencode::type::bs;
    const char* s_data = "spam";
    item_to_encode.data = buffer(s_data, s_data + strlen(s_data));
    buffer encoded_buf = bencode::encode(item_to_encode);
    ASSERT_EQ(std::string(encoded_buf.begin(), encoded_buf.end()), "4:spam");
}

// 测试编码列表
TEST(BencodeTest, EncodeList) {
    bencode::item item_to_encode;
    item_to_encode.t = bencode::type::l;
    std::vector<bencode::item> list_data;

    bencode::item int_item;
    int_item.t = bencode::type::i;
    int_item.data = 42LL;
    list_data.push_back(int_item);

    bencode::item str_item;
    str_item.t = bencode::type::bs;
    const char* s_data = "spam";
    str_item.data = buffer(s_data, s_data + strlen(s_data));
    list_data.push_back(str_item);

    item_to_encode.data = list_data;
    buffer encoded_buf = bencode::encode(item_to_encode);
    ASSERT_EQ(std::string(encoded_buf.begin(), encoded_buf.end()), "li42e4:spame");
}

// 测试编码字典
TEST(BencodeTest, EncodeDictionary) {
    bencode::item item_to_encode;
    item_to_encode.t = bencode::type::d;
    std::map<bencode::item, bencode::item> dict_data;

    bencode::item key1_item;
    key1_item.t = bencode::type::bs;
    const char* s_key1 = "key";
    key1_item.data = buffer(s_key1, s_key1 + strlen(s_key1));

    bencode::item value1_item;
    value1_item.t = bencode::type::i;
    value1_item.data = 42LL;

    dict_data[key1_item] = value1_item;

    bencode::item key2_item;
    key2_item.t = bencode::type::bs;
    const char* s_key2 = "spam";
    key2_item.data = buffer(s_key2, s_key2 + strlen(s_key2));
    
    bencode::item value2_list_item;
    value2_list_item.t = bencode::type::l;
    std::vector<bencode::item> inner_list_data;
    bencode::item inner_int1;
    inner_int1.t = bencode::type::i;
    inner_int1.data = 1LL;
    inner_list_data.push_back(inner_int1);
    bencode::item inner_int2;
    inner_int2.t = bencode::type::i;
    inner_int2.data = 2LL;
    inner_list_data.push_back(inner_int2);
    value2_list_item.data = inner_list_data;

    dict_data[key2_item] = value2_list_item;
    
    item_to_encode.data = dict_data;
    buffer encoded_buf = bencode::encode(item_to_encode);
    // Note: Dictionary keys are sorted alphabetically before encoding.
    // "key" comes before "spam"
    ASSERT_EQ(std::string(encoded_buf.begin(), encoded_buf.end()), "d3:keyi42e4:spamli1ei2ee");
}

// 测试解析无效的Bencode数据
TEST(BencodeTest, ParseInvalidBencode) {
    const char* s_invalid_start = "x42e";
    buffer buf_invalid_start(s_invalid_start, s_invalid_start + strlen(s_invalid_start)); // Invalid start
    ASSERT_THROW(bencode::parse(buf_invalid_start), bencode::invalid_bencode);

    const char* s_unterminated_int = "i42";
    buffer buf_unterminated_int(s_unterminated_int, s_unterminated_int + strlen(s_unterminated_int)); // Unterminated integer
    ASSERT_THROW(bencode::parse(buf_unterminated_int), bencode::invalid_bencode);

    const char* s_invalid_str_len = "4spam";
    buffer buf_invalid_str_len(s_invalid_str_len, s_invalid_str_len + strlen(s_invalid_str_len)); // Missing colon in string
    ASSERT_THROW(bencode::parse(buf_invalid_str_len), bencode::invalid_bencode);
    
    const char* s_incomplete_str = "5:spam";
    buffer buf_incomplete_str(s_incomplete_str, s_incomplete_str + strlen(s_incomplete_str)); // Incomplete string
    ASSERT_THROW(bencode::parse(buf_incomplete_str), bencode::invalid_bencode);
} 