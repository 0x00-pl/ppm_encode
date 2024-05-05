#ifndef BIT_STREAM_H
#define BIT_STREAM_H
#include<fstream>
using namespace std;


namespace pl{
	
typedef const unsigned char* src_iter;
typedef char int_8;
typedef short int_16;
typedef unsigned short uint_16;
typedef int int_32;
typedef unsigned int uint_32;
typedef unsigned int buff_type;

class bit_ostream{
public:
	bit_ostream(ofstream& s):sbind(s){
		next_bit_index=32;
		buff=0;
	}
	void push_bit_EX(char* src, size_t count){

	}
	// <-count-| 24MAX
	void push_bit(uint_32 src, size_t count){
		while(next_bit_index<=24){
			char write_temp= char(buff>>24);
			sbind.write(&write_temp,1);
			buff<<=8;
			next_bit_index+=8;
		}
		buff|= src<<(next_bit_index-count);
		next_bit_index-= count;
	}
	void close(){
		char write_temp= char(buff>>24);
		sbind.write(&write_temp,1);
		sbind.close();
	}
	buff_type buff;
	int next_bit_index;
	ofstream& sbind;
};
class bit_istream{
public:
	bit_istream(ifstream& s):sbind(s){
		next_bit_index=0;
		buff=0;
	}
	// <-count-| 24MAX
	uint_32 pop_bit(size_t count){
		uint_32 ret;
		while(next_bit_index<=24){
			buff<<=8;
			char temp=0;
			sbind.read(&temp,1);
			buff|= int(temp)&((1<<8)-1);
			next_bit_index+=8;
		}
		ret= (buff>>(next_bit_index-count))&((1<<count)-1);

		next_bit_index-= count;
		return ret;
	}
	bool eof(){
		return sbind.eof();
	}
	buff_type buff;
	int next_bit_index;
	ifstream& sbind;
};


}
#endif