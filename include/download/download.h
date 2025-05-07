#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <vector>
#include "tracker/tracker.h"
#include <queue>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "parsing/buffer.h"

// 从worker.h移入的speed类
class speed {
private:
	std::atomic<unsigned int> bytes, received;
	std::atomic<bool> finito;
	std::thread t;
	
	unsigned int prev;
	long long total;
	const double e_factor = 0.6321205588; // 1 - 1/e
	const int delay = 500;
	int mod;

	void human_readable(unsigned int b); 

public:
	speed(): total(0), bytes(0), finito(false), prev(0), mod(0) {}
	void set_total(long long t);
	void start();
	void stop();
	void add(unsigned int b);
	void draw(double progress, unsigned int speed);
};

// 从worker.h移入的writer类
class writer {
private:
	std::ofstream out;
	std::atomic<bool> finito;
	std::mutex mx;
	std::condition_variable cv;
	std::thread t;

	struct job {
		buffer b;
		int offset;
		job(buffer b, int offset): b(b), offset(offset) {};
	};

	std::queue<job> q;

public:
	writer(std::string filename): out(filename, std::ios::binary), finito(false) {}
	void start();
	void stop();
	void add(buffer& b, int offset);
};

class download {

private:
	std::vector<std::vector<bool>> received;
	std::vector<std::vector<bool>> is_in_queue;
	const std::vector<peer>& peers;
	torrent& t;

	writer w;
	speed s;

	int received_count;
	int total_blocks;

public:
	download(const std::vector<peer>& peers, torrent& t);
	void add_received(int piece, int block, buffer b);
	
	void start();
	double completed();
	bool is_done();

	struct job {
		int index;
		int begin;
		int length;

		int requested;

		job(int i, int b, int l): index(i), begin(b), length(l), requested(0) {}
		job() {}
		bool operator<(const job& other) const {return this->requested > other.requested;}
	};

	void push_job(job j);
	job pop_job();

	static const int BLOCK_SIZE = (1<<14);

private:
	std::priority_queue<job> jobs;
};

#endif // DOWNLOAD_H