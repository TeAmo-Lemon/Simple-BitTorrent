#ifndef URL_H
#define URL_H

#include <string>

struct url_t {

	std::string host;
	std::string path;
	int port;

	enum {UDP, HTTP, HTTPS} protocol;

	url_t(const std::string& url);
	url_t() {};
	bool operator==(const url_t& other) const;
};

#endif // URL_H
