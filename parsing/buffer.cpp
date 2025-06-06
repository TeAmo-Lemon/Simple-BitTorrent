#include "parsing/buffer.h"
#include <stdexcept>
#include <iostream>

using namespace std;

unsigned int getBE16(const buffer& b, buffer::size_type idx) {

	if(idx+1 >= b.size()) throw runtime_error("越界");

	return (b[idx] << 8) + b[idx+1];
}

unsigned int getBE32(const buffer& b, buffer::size_type idx) {

	if(idx+3 >= b.size()) throw runtime_error("越界");

	unsigned int ans = 0;
	for(int i=0;i<4;i++) {
		ans <<= 8;
		ans |= b[idx+i];
	}
	return ans;
}

buffer setBE32(unsigned int n, buffer& b, const buffer::size_type idx) {

	if(idx+3 >= b.size()) throw runtime_error("越界");

	for(int i=0;i<4;i++) {

		b[idx+3-i] = n & 0xff;
		n >>= 8;
	}
	return b;
}

buffer setBE16(unsigned int n, buffer& b, const buffer::size_type idx) {

	if(idx+1 >= b.size()) throw runtime_error("越界");

	b[idx+1] = n & 0xff;
	b[idx] = (n >> 8) & 0xff;
	return b;
}

void print(const buffer& b) {

	cout<<hex;
	for(unsigned char c:b){
		cout<<"0x"<<(unsigned int)c<<" ";
	}
	cout<<endl<<dec;
}
