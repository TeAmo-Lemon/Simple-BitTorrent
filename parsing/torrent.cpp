#include "parsing/torrent.h"
#include <iterator>
#include <fstream>
#include "parsing/bencode.h"
#include <openssl/sha.h>
#include <stdlib.h>
#include <iostream>
#include <any>

using namespace std;
using std::any_cast;

// 定义在 include/parsing/torrent.h 中声明的 static const 成员
const unsigned int torrent::BLOCK_SIZE;

torrent::torrent(const string& filename) {

	buffer buff = get_bytes(filename);
	bencode::item item = bencode::parse(buff);

	// 主announce URL
	this->url = item.get_string("announce");
	this->info_hash = get_hash_info(item);
	this->length = get_length(item);

	// 解析announce-list（如果存在）
	if (item.key_present("announce-list")) {
		vector<bencode::item> tierList = item.get_list("announce-list");
		for (auto& tier : tierList) {
			// 注意：这里我们假设tier是一个列表，但没有键值
			// 我们需要从bencode::item中提取列表
			if(tier.t == bencode::l) { // 检查它是否是一个列表类型
				vector<bencode::item>* trackers = any_cast<vector<bencode::item>>(&tier.data);
				if(trackers) {
					for (auto& tracker : *trackers) {
						try {
							// 检查tracker是否是字符串类型
							if(tracker.t == bencode::bs) {
								buffer* trackerBuffer = any_cast<buffer>(&tracker.data);
								if(trackerBuffer) {
									string trackerString(trackerBuffer->begin(), trackerBuffer->end());
									url_t trackerUrl(trackerString);
									
									// 避免重复
									bool isDuplicate = false;
									if (trackerUrl.host == this->url.host && 
										trackerUrl.port == this->url.port && 
										trackerUrl.path == this->url.path &&
										trackerUrl.protocol == this->url.protocol) {
										isDuplicate = true;
									}
									
									for (const auto& existingUrl : this->announce_list) {
										if (trackerUrl.host == existingUrl.host && 
											trackerUrl.port == existingUrl.port && 
											trackerUrl.path == existingUrl.path &&
											trackerUrl.protocol == existingUrl.protocol) {
											isDuplicate = true;
											break;
										}
									}
									
									if (!isDuplicate) {
										this->announce_list.push_back(trackerUrl);
									}
								}
							}
						} catch (const exception& e) {
							// 如果解析单个Tracker URL失败，继续下一个
							cerr << "无法解析Tracker URL: " << e.what() << endl;
						}
					}
				}
			}
		}
	}

	bencode::item info = item.get_item("info");

	this->piece_length = info.get_int("piece length");

	buffer hashes = info.get_buffer("pieces");

	const int SHA1_SIZE = 20;
	this->pieces = hashes.size() / SHA1_SIZE;

	buffer name_bytes = info.get_buffer("name");
	this->name = string(name_bytes.begin(), name_bytes.end());
}

buffer torrent::get_bytes(const string& filename) {

	ifstream is;
	buffer bytes;

	is.open(filename, ios::binary);
	if(is.fail()) {
		throw runtime_error("打开文件失败");
	}

	is.seekg(0, ios::end);
	size_t filesize=is.tellg();
	is.seekg(0, ios::beg);

	bytes.reserve(filesize);
	bytes.assign(istreambuf_iterator<char>(is),
	                     istreambuf_iterator<char>());

	return bytes;
}

buffer torrent::get_hash_info(const bencode::item& item) {

	bencode::item info = item.get_item("info");
	buffer encoded = bencode::encode(info);

	const int SIZE_SHA1 = 20;

	unsigned char buff[SIZE_SHA1];
	SHA1(encoded.data(), encoded.size(), buff);

	return buffer(buff,buff+SIZE_SHA1);
}

long long torrent::get_length(const bencode::item& item) {

	long long length = 0;

	bencode::item info = item.get_item("info");
	if(info.key_present("files")) {

		cout<<"目前，应用程序只处理单个文件的种子"<<endl;
		exit(0);

		vector<bencode::item> files = info.get_list("files");
		for (bencode::item& file:files) {
			length += file.get_int("length");
		}

	} else {

		length = info.get_int("length");
	}

	return length;
}

unsigned int torrent::get_piece_length(unsigned int piece) {

	return piece == pieces - 1 ? 
		(length % piece_length == 0 ? piece_length : length % piece_length) : 
		piece_length;
}

unsigned int torrent::get_n_blocks(unsigned int piece) {

	return (get_piece_length(piece) + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

unsigned int torrent::get_block_length(unsigned int piece, unsigned int block_index) {

	return block_index == get_n_blocks(piece) - 1 ? 
			(get_piece_length(piece) % BLOCK_SIZE == 0 ? 
					BLOCK_SIZE : 
					get_piece_length(piece) % BLOCK_SIZE) : 
			BLOCK_SIZE;
}


