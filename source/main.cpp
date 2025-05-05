#include "parsing/torrent.h"
#include <iostream>
#include "download/peer_id.h"
#include "tracker/tracker.h"
#include "download/download.h"

using namespace std;

int main(int argc, const char** argv) {

	if(argc < 2) {
		cout<<"使用方法: BitTorrent <torrent_file>"<<endl;
		return 0;
	}

	srand(time(NULL));
	peer_id::generate();

	torrent t(argv[1]);

	cout<<"正在从tracker获取peers... 耐心等待"<<endl;
	vector<peer> v = tracker::get_peers(t);
	cout<<"收到 "<<v.size()<<" peers"<<endl;

	download d(v,t);
	d.start();

	cout<<endl<<"成功下载"<<endl;

	return 0;
}
