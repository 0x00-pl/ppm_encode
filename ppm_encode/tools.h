#ifndef TOOLS_H
#define TOOLS_H
#include<sstream>
#include<vector>
#include<algorithm>
namespace pl{

template <typename T>
std::basic_string<T> ss_itoa(long n,unsigned w=0){
	std::basic_stringstream<T> stream;
	if (w){
		stream.fill('0');
		stream.width(w);
	}
	stream <<n;
	return stream.str();
}

template<typename Tkey, typename Tvalue>
class value_pair: public pair<Tkey,Tvalue>{
public:
	value_pair(const Tkey& key, const Tvalue& value){
		first= key;
		second= value;
	}
	operator Tkey(){ return first; }
	bool operator ==(const value_pair& other){
		return other.first== first;
	}
};

template<typename Tkey, typename Tvalue>
class vector_map: public vector<value_pair<Tkey, Tvalue> >{
public:
	vector_map(){
		is_sort= false;
		is_uniquq= false;
	}
	void map_sort(){
		if(is_sort==true) return;
		sort(begin(), end());
		is_sort= true;
	}
	void map_unique(){
		if(is_uniquq==true) return;
		map_sort();
		//保留最后一次更改
		erase(begin(), unique(rbegin(),rend()).base());
		//erase(unique(begin(),end()), end());
		is_uniquq= true;
	}
	void map_insert(const Tkey& key, const Tvalue& value){
		is_sort=is_uniquq= false;
		this->push_back(value_pair<Tkey,Tvalue>(key, value));
	}
	iterator map_find(const Tkey& key){
		map_unique();
		return lower_bound(begin(), end(), key);
	}
	bool is_sort;
	bool is_uniquq;
};




}
#endif