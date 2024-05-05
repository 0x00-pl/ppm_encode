#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<fstream>
#include<iomanip> 
#include"bit_stream.h"
#include"haffman.h"
using namespace std;

namespace pl{

typedef const unsigned char* src_iter;
typedef char int_8;
typedef short int_16;
typedef unsigned short uint_16;
typedef int int_32;
typedef unsigned int uint_32;
typedef unsigned int buff_type;

const size_t G_dic_size= 512; 





class element{
public:
	vector<src_iter> next_element;
};
element element_table[256];
void output_element(){
	for(int i=0; i<256; ++i){
		cout<<i<<" :"<<endl;
		element& temp= element_table[i];
		for(size_t j=0; j< temp.next_element.size(); ++j){
			cout<<"   ..."<<string(temp.next_element[j]-9, temp.next_element[j]+1)<<endl;
		}
	}
}
void cleanup_element(){
	for(int i=0; i<256; ++i){
		element_table[i].next_element.clear();
	}
}

size_t str_near(src_iter from, src_iter to, int step){
	if(from==to){return UINT_MAX;}
	size_t len=0;
	while(*from==*to){
		from+= step;
		to+= step;
		++len;
	}
	return len;
}

bool _reorder_near_tab_mt(pair<size_t,char>& i, pair<size_t,char>& j){ return (j.first<i.first); }

void reorder(element& src, src_iter comp, char dest[256]){
	pair<size_t, char> near_tab[256];
	for(int i=0; i<256; ++i){
		near_tab[i].first= 0;
		near_tab[i].second= i;
	}
	//cout<<src.next_element.size()<<endl;
	for(size_t i=0; i< src.next_element.size(); ++i){
		size_t near_length= 0;
		if(src.next_element[i]!=comp){
			near_length= str_near(src.next_element[i]-1, comp, -1);
		}
		if(near_length > near_tab[*src.next_element[i]].first){
			near_tab[*src.next_element[i]].first= near_length;
		}
	}
	sort(near_tab, near_tab+256, _reorder_near_tab_mt);//cout<<"maxlen="<<near_tab[0].first<<endl;
	//cout<<"sorted"<<near_tab[*comp].first<<near_tab[*comp].first
	for(int i=0; i<256; ++i) dest[i]= near_tab[i].second;
}



void encode(src_iter src, int_8* dest, size_t buff_size){
	char ordered[256];
	dest[0]= src[0];
	for(size_t i=1; i<buff_size; ++i){
		src_iter cur_src= src+i;
		int_8* cur_dest= dest+i;
		int prev_src_char= cur_src[-1];

		reorder(element_table[prev_src_char&(1<<8)-1], cur_src-1, ordered);
		char* res= find(ordered, ordered+256, (char)*cur_src);
		*cur_dest= res-ordered;
		auto& temp_p_pool= element_table[prev_src_char&(1<<8)-1].next_element;
		if(temp_p_pool.size()>G_dic_size){
			temp_p_pool.erase(temp_p_pool.begin(), temp_p_pool.begin()+G_dic_size/2);
		}
		temp_p_pool.push_back(cur_src);
	}
}

void decode(int_8* src, unsigned char* dest, size_t buff_size){
	char ordered[256];
	dest[0]= src[0];
	for(size_t i=1; i<buff_size; ++i){
		int_8* cur_src= src+i;
		unsigned char* cur_dest= dest+i;
		int prev_dest_char= cur_dest[-1];

		reorder(element_table[prev_dest_char&(1<<8)-1], cur_dest-1, ordered);
		int unsigned_src= (*cur_src)&((1<<8)-1);
		*cur_dest= ordered[unsigned_src];
		auto& temp_p_pool= element_table[prev_dest_char&(1<<8)-1].next_element;
		if(temp_p_pool.size()>G_dic_size){
			temp_p_pool.erase(temp_p_pool.begin(), temp_p_pool.begin()+G_dic_size/2);
		}
		temp_p_pool.push_back(cur_dest);
	}
}

void encode_file(string file_name, string pkg_file_name){
	ifstream in_file(file_name, ifstream::binary|ifstream::in);
	if(!in_file.is_open()){cout<<"bad file"<<endl;return;}
	in_file.seekg(0, ios_base::end);
	uint_32 file_length= in_file.tellg();

	in_file.seekg(0, ios_base::beg);
	unsigned char* buff= new unsigned char[file_length];
	in_file.read((char*)buff,file_length);
	in_file.close();

	int_8* code= new int_8[file_length];
	
	cleanup_element();
	encode(buff, code, file_length);

	int count= 0;
	int times[256]={0};
	char char_set[256];
	for(int i=0; i<256; ++i) char_set[i]=i;

	for(size_t i=0; i<file_length; ++i){
		++times[index_char(code[i])];
		++count;
	}
	//归一化
	for(size_t i=0; i<256; ++i){
		float f= float(times[i])/count;
		times[i]= f*65536;
	}

	haffman_tree hafm(char_set, times, 256);


	//输出文件
	ofstream out_file(pkg_file_name, ifstream::binary|ifstream::out);
	//输出原始文件大小
	out_file.write((char*)&file_length,4);

	//put code
	bit_ostream bo(out_file);

	//haffman 头文件
	hafm.save_tree_Tchar(bo);
	//haffman 编码
	hafm.encode(code, bo, file_length);

	//输出中间文件
	ofstream oomid("omid.txt", ios::binary);
	oomid.write((char*)code, file_length);
	oomid.close();

	bo.close();

	delete code;
	delete buff;
}

void decode_file(string file_name, string pkg_file_name){
	ifstream in_file(pkg_file_name, ifstream::binary|ifstream::in);
	if(!in_file.is_open()){cout<<"bad file"<<endl;return;}
	//pop size
	uint_32 file_length;
	//输入原始文件大小
	in_file.read((char*)&file_length,4);

	int_8* code= new int_8[file_length];
	unsigned char* buff= new unsigned char[file_length];

	//pop code
	bit_istream bi(in_file);
	
	//haffman 头文件
	haffman_tree hafm(bi);
	

	//haffman 编码
	hafm.decode(bi, code, file_length);



	
	//输出中间文件
	ofstream oomid("omid2.txt", ios::binary);
	oomid.write(code, file_length);
	oomid.close();

	in_file.close();

	
	cleanup_element();
	decode(code,buff,file_length);

	ofstream out_file(file_name, ifstream::binary|ifstream::out);
	out_file.write((char*)buff, file_length);
	out_file.close();

	delete code;
	delete buff;
}



void file_echo(){
	ifstream in_file("1.txt", ifstream::binary|ifstream::in);
	ofstream out_file("1_echo.txt", ofstream::binary|ofstream::out);
	
	bit_istream bi(in_file);
	bit_ostream bo(out_file);
	//while (in_file.good())
	//	out_file<<(char)in_file.get();
	while(!bi.eof()){
		int_16 buf= bi.pop_bit(8);
		buf<<=8;
		bo.push_bit(buf,8);
	}
}


//int f_cl_file(string file_name, int times[256]){
//	int count=0;
//	ifstream in_file(file_name, ifstream::binary|ifstream::in);
//	while(in_file.good()){
//		int index= in_file.get()&(1<<8)-1;
//		++times[index];
//		++count;
//	}
//	
//	for(int i=0; i<256; ++i){
//		if(i%8==0){cout<<endl;}
//		cout<<i<<":"<<times[i]<<"  ";
//		//if(times[i]!=0){cout.setf(ios::fixed);
//		//	cout<<i<<setprecision(3)<<":"<<(float)times[i]/count<<"%  ";
//		//}
//		//else
//		//	cout<<i<<":0.00%  ";
//	}
//	return count;
//}


}

void main(){
	//pl::file_echo();
	string input;
	cout<<"encode/decode"<<endl;
	cin>>input;
	if(input=="encode"){
		cout<<"from to"<<endl;
		string from,to;
		cin>>from;
		cin>>to;
		pl::encode_file(from, to);
	}
	if(input=="decode"){
		cout<<"from to"<<endl;
		string from,to;
		cin>>from;
		cin>>to;
		pl::decode_file(from, to);
	}
	system("pause");
}