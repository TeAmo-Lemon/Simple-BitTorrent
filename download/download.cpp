#include "download/download.h"
#include <thread>
#include "download/peer_connection.h"
#include <algorithm>
#include <cassert>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <vector>

using namespace std;

// writer类方法实现（从worker.cpp移入）
void writer::start() {
	t = thread([this](){
		while(!finito) {
			unique_lock<mutex> lock(mx);
			cv.wait(lock, [this](){
				return !q.empty() || finito;
			});

			vector<job> vec;

			while(!q.empty()) {
				job j = q.front();
				q.pop();
				vec.push_back(move(j));
			}

			lock.unlock();

			for(job& j:vec) {
				out.seekp(j.offset, ios::beg);
				out.write(reinterpret_cast<char*>(j.b.data()), j.b.size());
			}
		}
	});
}

void writer::stop() {
	finito = true;
	cv.notify_one();
	t.join();
}

void writer::add(buffer& b, int offset) {
	unique_lock<mutex> lock(mx);
	q.push(job(move(b), offset));
	lock.unlock();
	cv.notify_one();
}

// speed类方法实现（从worker.cpp移入）
void speed::start() {
	cout<<"Wait for the download to complete ..."<<endl;
	draw(0.0, 0);

	t = thread([this](){
		while(!finito) {
			if(mod % 5 == 0) {
				unsigned int v = bytes.exchange(0);
				unsigned int speed = 1000 * v / delay;
				prev = e_factor * speed + (1.0 - e_factor) * prev;
			}
			mod = (mod + 1) % 5;
			
			draw((double)received / total, prev);
			this_thread::sleep_for(chrono::milliseconds(delay / 5));
		}
		draw((double)received / total, 0.0);
	});
}

void speed::stop() {
	finito = true;
	t.join();
}

void speed::add(unsigned int b) {
	bytes += b;
	received += b;
}

void speed::draw(double progress, unsigned int speed) {
	int bar_width = 50;

	cout << "[";
	int pos = bar_width * progress;
	for (int i = 0; i < bar_width; ++i) {
		if (i < pos) cout << "=";
		else if (i == pos) cout << ">";
		else cout << " ";
	}
	cout << "] " << int(progress * 100.0) << " %   ";
	human_readable(speed);
	cout<<"              \r";
	cout.flush();
}

void speed::set_total(long long t) {
	total = t;
}

void speed::human_readable(unsigned int b) {
	if (b < 1024) cout<<b<<" B/s";
	else if (b < 1024 * 1024) cout<<(double)b/1024<<" kB/s";
	else if (b < 1024 * 1024 * 1024) cout<<(double)b/(1024*1024)<<" Mb/s";
	else cout<<(double)b/(1024*1024*1024)<<" Gb/s";
}

// download类方法实现
download::download(const vector<peer>& peers, torrent& t): 
	t(t), peers(peers), received_count(0), w(t.name) {

	received = vector<vector<bool>>(t.pieces);
	is_in_queue = vector<vector<bool>>(t.pieces);

	total_blocks = 0;

	for(int i=0;i<t.pieces;i++) {
		int n = t.get_n_blocks(i);
		total_blocks += n;
		is_in_queue[i] = vector<bool>(n);
		received[i] = vector<bool>(n);
	}

	s.set_total(total_blocks * BLOCK_SIZE);
}

void download::start() {
	if(peers.size() == 0) throw runtime_error("no peers");

	w.start();
	s.start();

	vector<peer_connection> conns;
	for(const peer& p: peers) {
		try {
			conns.push_back(peer_connection(p, t, *this));
		} catch (exception& e) {
			cout<<"connection constructor threw: "<<e.what()<<endl;
		}
	}

	connection_farm f(conns, *this);
	f.hatch();

	w.stop();
	s.stop();
}

void download::add_received(int piece, int block, buffer piece_data) {
	assert(piece >= 0 && piece < t.pieces);
	assert(block >= 0 && block < t.get_n_blocks(piece));

	if(received[piece][block]) return;

	int offset = block * BLOCK_SIZE;
	s.add(piece_data.size());
	w.add(piece_data, piece * t.piece_length + offset);

	received_count++;
	received[piece][block] = true;
}

bool download::is_done() {
	bool is_done = received_count == total_blocks;

	auto lambda = [this](){
		bool all_true = true;
		for( auto v:received) {
			for(bool x:v){
				all_true = all_true && x;
			}
		}
		return all_true;
	};	

	if (is_done) {
		assert(lambda());
	}

	return is_done;
}

double download::completed() {
	return (double)received_count / total_blocks;
}

void download::push_job(job j) {
	assert(j.index >= 0 && j.index < t.pieces);
	assert(j.begin % BLOCK_SIZE == 0);
	assert(j.begin / BLOCK_SIZE >= 0 && j.begin / BLOCK_SIZE < t.get_n_blocks(j.index));
	assert(0 < j.length && j.length <= BLOCK_SIZE);

	if (is_in_queue[j.index][j.begin / BLOCK_SIZE]) {
		return;
	}

	is_in_queue[j.index][j.begin / BLOCK_SIZE] = true;
	jobs.push(j);
}

download::job download::pop_job() {
	while(!jobs.empty()) {
		job j = jobs.top();
		jobs.pop();
		if(!received[j.index][j.begin / BLOCK_SIZE]) {
			j.requested++;
			jobs.push(j);
			return j;
		}
	}

	throw exception();
}

