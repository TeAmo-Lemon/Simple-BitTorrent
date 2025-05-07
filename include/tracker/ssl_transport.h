#ifndef SSL_TRANSPORT_H
#define SSL_TRANSPORT_H

#include "tracker/transport.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>

class ssl_transport: public transport {
private:
    SSL_CTX* ctx;
    SSL* ssl;
    bool ssl_connected;
    
    void init_ssl();
    void cleanup_ssl();

public:
    ssl_transport(std::string address, int port, bool blocking = true);
    ~ssl_transport() override;
    
    void connect_ssl();
    void send(buffer message) override;
    buffer receive() override;
    void close() override;
};

#endif // SSL_TRANSPORT_H 