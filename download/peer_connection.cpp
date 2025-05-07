#include "download/peer_connection.h"
#include "download/message.h"
#include "parsing/buffer.h"
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <unistd.h>
#include <string.h>
#include <iostream>

using namespace std;

// peer_connection 类的实现 (原connection)
peer_connection::peer_connection(const peer& p, torrent& t, download& d): 
    p(p), d(d), t(t), buff(buffer()), 
    handshake(true), choked(true), connected(false), 
    socket(p.host, p.port, false) {}

void peer_connection::handle(buffer& msg) {
    if (handshake) {
        handshake = false;
        socket.send(message::build_interested());
    } else if (msg.size() < 4) {
        throw runtime_error("消息长度小于4字节");
    } else if (msg.size() == 4) {
        // keep alive message
    } else {
        switch (msg[4]) {
            case 0:
                choke_handler();
                break;
            case 1:
                unchoke_handler();
                break;
            case 4:
                have_handler(msg);
                break;
            case 5:
                bitfield_handler(msg);
                break;
            case 7:
                piece_handler(msg);
                break;
            default:
                break;
        }
    }
}

void peer_connection::choke_handler() {
    socket.close();
}

void peer_connection::unchoke_handler() {
    choked = false;
    request_piece();
}

void peer_connection::have_handler(buffer& b) {
    unsigned int piece = getBE32(b,5);
    if(piece >= t.pieces) 
        throw runtime_error("收到的have消息包含无效的piece索引");

    enqueue(piece);
    request_piece();
}

void peer_connection::bitfield_handler(buffer& b) {
    unsigned int n_bytes = getBE32(b,0) - 1;
    if(n_bytes != (t.pieces + 7) / 8) 
        throw runtime_error("bitfield字节数不正确");
    
    for(int i=0;i<n_bytes;i++) {
        unsigned char byte = b[5+i];

        for(int j=0;j<8;j++) {
            if(byte & (1<<(7-j))) {
                enqueue(i*8 + j);
            }
        }
    }

    request_piece();
}

void peer_connection::request_piece() {
    if(choked) return;

    download::job j;

    try {
        j = d.pop_job();
    } catch(...) {
        socket.close();
        return;
    }

    assert(j.begin % download::BLOCK_SIZE == 0);
    socket.send(message::build_request(j.index, j.begin, j.length));
}

void peer_connection::piece_handler(buffer& b) {
    unsigned int block_size = getBE32(b, 0) - 9;
    if (block_size > download::BLOCK_SIZE) {
        throw runtime_error("收到的block大小不正确");
    }

    unsigned int index = getBE32(b, 5);
    unsigned int begin = getBE32(b, 9);

    b.erase(b.begin(), b.begin() + 13);
    
    // Todo verify hash corresponds

    assert(b.size() == block_size);
    if (begin % download::BLOCK_SIZE != 0) {
        throw runtime_error("收到的begin偏移量不正确");
    }

    d.add_received(index, begin / download::BLOCK_SIZE, b);
    request_piece();
}

void peer_connection::ready() {
    if (!connected) {
        connected = true;
        socket.send(message::build_handshake(t));
        return;
    }

    auto length = [this](){
        return handshake ? buff[0] + 49 : getBE32(buff,0) + 4;
    };

    buffer b = socket.receive();
    copy(b.begin(), b.end(), back_inserter(buff));

    if (buff.size() >= 4 && buff.size() >= length()) {
        buffer msg(buff.begin(), buff.begin() + length());
        buff.erase(buff.begin(), buff.begin() + length());

        handle(msg);
    }
}

void peer_connection::enqueue(int piece) {
    assert(piece < t.pieces);

    int n_blocks = t.get_n_blocks(piece);
    for(int i=0;i<n_blocks;i++) {
        d.push_job(download::job(piece, i*download::BLOCK_SIZE, t.get_block_length(piece, i)));
    }
}

// connection_farm 类的实现 (原farm)
connection_farm::connection_farm(vector<peer_connection>& conns, download& d): conns(conns), d(d) {
    epfd = epoll_create(1);
    if (epfd < 0) {
        string what = strerror(errno);
        throw runtime_error(what);
    }
}

void connection_farm::hatch() {
    for(int i=0;i<conns.size();i++) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP;
        ev.data.u32 = i;
        int res = epoll_ctl(epfd, EPOLL_CTL_ADD, conns[i].socket.fd, &ev);

        if (res < 0) {
            string what = strerror(errno);
            throw runtime_error(what);
        }
    }

    while(!d.is_done()) {
        int nfd = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfd < 0) {
            string what = strerror(errno);
            throw runtime_error(what);
        }

        for(int i=0;i<nfd;i++) {
            int mask = events[i].events;
            int idx = events[i].data.u32;

            if(mask & (EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP)) {
                conns[idx].socket.close();
                // Todo retry to connect the socket here
            } else if (mask & (EPOLLOUT | EPOLLIN)) {
                // Todo put this in a worker thread
                try {
                    conns[idx].ready();
                } catch (exception& e) {
                    cout<<"ready()抛出异常: "<<e.what()<<endl;
                }

                if (mask & EPOLLOUT) {
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP;
                    ev.data.u32 = idx;

                    int res = epoll_ctl(epfd, EPOLL_CTL_MOD, conns[idx].socket.fd, &ev);
                    if (res < 0) {
                        string what = strerror(errno);
                        throw runtime_error(what);
                    }
                }
            } else {
                throw runtime_error("未知的ready事件掩码");
            }
        }
    }

    if (::close(epfd) < 0) {
        string what = strerror(errno);
        throw runtime_error(what);
    }
} 