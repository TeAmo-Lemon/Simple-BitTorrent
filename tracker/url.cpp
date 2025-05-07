#include "tracker/url.h"
#include <stdexcept>
#include <algorithm>

using namespace std;

url_t::url_t(const string& url) {

	string s = url;

	string pref_udp = "udp://";
	string pref_http = "http://";
	string pref_https = "https://";

	// udp://tracker.coppersurfer.tk:6969/announce

	if(s.substr(0,pref_udp.size()) == pref_udp) {
		s.erase(0,pref_udp.size());
		this->protocol = UDP;

	} else if(s.substr(0,pref_http.size()) == pref_http) {
		s.erase(0,pref_http.size());
		this->protocol = HTTP;
        
	} else if(s.substr(0,pref_https.size()) == pref_https) {
		s.erase(0,pref_https.size());
		this->protocol = HTTPS;

	} else {
		throw runtime_error("undefined protocol in url");
	}
	
	// 查找冒号，用于分割主机名和端口号
	auto colon_it = find(s.begin(), s.end(), ':');
	
	// 查找第一个斜杠，用于分割主机+端口部分和路径部分
	auto slash_it = find(s.begin(), s.end(), '/');
	
	// 提取主机名
	string host;
	copy(s.begin(), colon_it != s.end() ? colon_it : slash_it, back_inserter(host));
	this->host = move(host);
	
	// 提取端口号（如果存在）
	int port;
	if (colon_it != s.end() && colon_it < slash_it) {
		// 端口号存在
		string port_str;
		copy(colon_it + 1, slash_it, back_inserter(port_str));
		port = stoi(port_str);
	} else {
		// 没有显式端口号，使用默认端口
		if (this->protocol == HTTP) {
			port = 80;
		} else if (this->protocol == HTTPS) {
			port = 443;
		} else if (this->protocol == UDP) {
			port = 6969; // 常见的UDP tracker端口
		} else {
			port = 80; // 默认
		}
	}
	this->port = port;
	
	// 提取路径
	string path;
	if (slash_it != s.end()) {
		copy(slash_it, s.end(), back_inserter(path));
	} else {
		path = "/"; // 默认路径
	}
	this->path = move(path);
}