#ifndef HTTPS_H
#define HTTPS_H

#include "parsing/buffer.h"
#include "tracker/ssl_transport.h"
#include "tracker/url.h"
#include <string>

class https {

private:
	ssl_transport socket;
	url_t url;
	buffer path;
	int n_args;

	void append(const std::string& s, buffer& b);
	void append(const buffer& s, buffer& b);

public:
	https(const url_t& url);
	std::string urlencode(const buffer& b);
	void add_argument(const std::string& key, const std::string& value);
	void add_argument(const std::string& key, const buffer& value);
	buffer get();
};

#endif // HTTPS_H 