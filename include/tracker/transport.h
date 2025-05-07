#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <netinet/in.h>
#include <vector>
#include <string>
#include "parsing/buffer.h"

class transport {

private:
	struct sockaddr_in servaddr; 

protected:
	static const int MAXLINE = 1500;
	char buff[MAXLINE];
	bool closed_flag;
	transport(std::string address, int port, int type, bool blocking = true);

public:
	virtual void send(buffer message);
	virtual buffer receive();
	int fd;
	virtual void close();
	bool closed();
	virtual ~transport() = default;
};

#endif // TRANSPORT_H
