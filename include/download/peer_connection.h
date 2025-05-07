#ifndef PEER_CONNECTION_H
#define PEER_CONNECTION_H

#include "parsing/torrent.h"
#include "tracker/tracker.h"
#include "parsing/buffer.h"
#include "tracker/tcp.h"
#include <vector>
#include "download/download.h"
#include <sys/epoll.h>

// 原connection类，与peer通信
class peer_connection {
private:
    void handle(buffer& msg);

    void choke_handler();
    void unchoke_handler();
    void have_handler(buffer& b);
    void bitfield_handler(buffer& b);
    void piece_handler(buffer& b);
    void enqueue(int piece);

    void request_piece();

    buffer buff;
    const peer& p;
    torrent& t;
    bool choked;
    bool handshake;
    bool connected;
    download& d;

public:
    tcp socket;
    peer_connection(const peer& p, torrent& t, download& d);
    void ready();
};

// 原farm类，管理多个peer_connection
class connection_farm {
private:
    int epfd;
    std::vector<peer_connection>& conns;
    download& d;

    static const int MAX_EVENTS = 1024;
    struct epoll_event events[MAX_EVENTS];

public:
    connection_farm(std::vector<peer_connection>& conns, download& d);
    void hatch();
};

#endif // PEER_CONNECTION_H 