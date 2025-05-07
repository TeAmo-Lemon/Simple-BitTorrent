#include "tracker/ssl_transport.h"
#include <stdio.h>
#include <stdexcept>
#include <string.h>
#include <unistd.h>

using namespace std;

void ssl_transport::init_ssl() {
    // 初始化OpenSSL库
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    // 创建SSL上下文
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        throw runtime_error("创建SSL上下文失败");
    }

    // 设置SSL验证选项 - 生产环境中应该进行严格验证
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    
    // 创建SSL连接
    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        SSL_CTX_free(ctx);
        throw runtime_error("创建SSL连接失败");
    }
    
    // 将SSL连接与socket关联
    if (SSL_set_fd(ssl, fd) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        throw runtime_error("SSL与socket关联失败");
    }
    
    ssl_connected = false;
}

void ssl_transport::cleanup_ssl() {
    if (ssl != NULL) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = NULL;
    }
    
    if (ctx != NULL) {
        SSL_CTX_free(ctx);
        ctx = NULL;
    }
}

ssl_transport::ssl_transport(string address, int port, bool blocking)
    : transport(address, port, SOCK_STREAM, blocking), ssl(NULL), ctx(NULL), ssl_connected(false) {
    init_ssl();
}

ssl_transport::~ssl_transport() {
    cleanup_ssl();
}

void ssl_transport::connect_ssl() {
    if (ssl_connected) return;
    
    int ret = SSL_connect(ssl);
    if (ret != 1) {
        int err = SSL_get_error(ssl, ret);
        char error_string[256];
        ERR_error_string_n(err, error_string, sizeof(error_string));
        throw runtime_error(string("SSL连接失败: ") + error_string);
    }
    
    ssl_connected = true;
}

void ssl_transport::send(buffer message) {
    if (!ssl_connected) {
        connect_ssl();
    }
    
    int written = SSL_write(ssl, message.data(), message.size());
    if (written <= 0) {
        int err = SSL_get_error(ssl, written);
        char error_string[256];
        ERR_error_string_n(err, error_string, sizeof(error_string));
        throw runtime_error(string("SSL写入失败: ") + error_string);
    }
}

buffer ssl_transport::receive() {
    if (!ssl_connected) {
        connect_ssl();
    }
    
    char buff[MAXLINE];
    int bytes = SSL_read(ssl, buff, MAXLINE);
    if (bytes <= 0) {
        int err = SSL_get_error(ssl, bytes);
        char error_string[256];
        ERR_error_string_n(err, error_string, sizeof(error_string));
        throw runtime_error(string("SSL读取失败: ") + error_string);
    }
    
    return buffer(buff, buff + bytes);
}

void ssl_transport::close() {
    if (closed_flag) {
        return;
    }
    
    cleanup_ssl();
    
    if (::close(fd) < 0) {
        string what = strerror(errno);
        throw runtime_error(what);
    }
    
    closed_flag = true;
} 