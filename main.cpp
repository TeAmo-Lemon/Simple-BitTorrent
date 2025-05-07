#include "parsing/torrent.h"
#include <iostream>
#include "download/message.h"
#include "tracker/tracker.h"
#include "download/download.h"

using namespace std;

int main(int argc, const char** argv) {

	if(argc < 2) {
		cout<<"使用方法: BitTorrent <torrent_file>"<<endl;
		cout<<"比如: ./BitTorrent sample.torrent"<<endl;
		return 0;
	}

	srand(time(NULL));
	peer_id::generate();

	// 打印peer_id
	cout<< "生成随机的peer_id: "<<endl;
	cout<<"peer_id: "<<endl;
	for(auto c:peer_id::get()) {
		cout<<c;
	}
	cout<<endl;

	torrent t(argv[1]);

	cout<<"正在从tracker获取peers... "<<endl;
	vector<peer> v = tracker::get_peers(t);
	cout<<"收到 "<<v.size()<<" 个peers"<<endl;

	download d(v,t);
	d.start();

	cout<<endl<<"成功下载文件"<<endl;

	return 0;
}
