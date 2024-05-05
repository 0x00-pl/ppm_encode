#ifndef HAFFMAN_H
#define HAFFMAN_H
#include"bit_stream.h"
#include<vector>
using namespace std;

namespace pl{

unsigned int index_char(char c){
	unsigned int ret= c;
	ret&= (1<<8)-1;
	return ret;
}


template<typename Tvalue>
class haffman_node{
public:
	typedef haffman_node<Tvalue> _Myt;
	haffman_node(const Tvalue& v, int _weight){
		value= v;
		weight= _weight;
		_0=_1=nullptr;
	}
	haffman_node(_Myt* _0_side, _Myt* _1_side){
		_0= _0_side;
		_1= _1_side;
		if(_0!=nullptr&&_1!=nullptr) weight= _0->weight+_1->weight;
	}
	~haffman_node(){
		//delete nullptr is safe
		delete _0;
		_0=nullptr;
		delete _1;
		_1=nullptr;
	}
	bool leaf(){return _0==nullptr && _1==nullptr;}
	_Myt* _0;
	_Myt* _1;
	Tvalue value;
	int weight;
};

class haffman_tree{
public:
	haffman_tree(char* val, int* weight, size_t size){
		max_depth=0;
		tree_root=nullptr;
		if(size==0) return;
		
		vector<haffman_node<char>*> pool;
		for(size_t i=0; i<size; ++i){
			pool.push_back(new haffman_node<char>(val[i], weight[i]));
		}
		while(pool.size()>=2){
			//ð�ݳ���С�������ϲ���һ��
			//����ʹ�� <= ��Ȩֵ��ͬ������� ����Щ����ٵ��Ⱥϲ�
			for(size_t i=0; i<pool.size()-1; ++i){
				if(pool[i]->weight <= pool[i+1]->weight){
					swap(pool[i], pool[i+1]);
				}
			}
			for(size_t i=0; i<pool.size()-2; ++i){
				if(pool[i]->weight <= pool[i+1]->weight){
					swap(pool[i], pool[i+1]);
				}
			}
			
			haffman_node<char>* _0_node= pool.back();
			pool.pop_back();
			haffman_node<char>* _1_node= pool.back();
			pool.pop_back();

			pool.push_back(new haffman_node<char>(_0_node, _1_node));
		}
		//����ʣ��һ����ʱ��
		tree_root= pool.back();
		fill_rmap(tree_root);
	}
	haffman_tree(bit_istream& from){
		max_depth=0;
		tree_root=nullptr;
		load_tree_Tchar(from);
	}
	~haffman_tree(){delete tree_root;}
	void save_tree_Tchar(bit_ostream& to){
		//1byte Ҷ�ڵ����
		to.push_bit(max_depth,8);
		
		//����*256 bit ����
		for(int i=0; i<256; ++i){
			int temp= haffman_rmap[i].first;
			temp<<= max_depth-haffman_rmap[i].second;
			to.push_bit(temp, max_depth);
		}
	}
	void load_tree_Tchar(bit_istream& from){
		//1byte Ҷ�ڵ����
		max_depth= index_char(from.pop_bit(8));
		//����*256 bit ����
		for(int i=0; i<256; ++i){
			haffman_rmap[i].first= from.pop_bit(max_depth);
			//����haffman����
			haffman_rmap[i].second=-1;
			add_leaf(haffman_rmap[i].first, i, &tree_root);
		}
	}

	void add_leaf(int dat, char val, haffman_node<char>** tree, int depth=0){
		//�½�Ҷ�ڵ�ʱ
		//����ʣ���ֽ�
		//����Ȩ��

		if(*tree==nullptr){
			auto _cur= new haffman_node<char>(val, -1);
			*tree= _cur;
		}else{
			if((*tree)->leaf()){
				//Ҷ�ڵ�Ͱѵ�ǰ�ļ���һ��ȥ
				haffman_node<char>* last= *tree;
				haffman_node<char>* _cur= new haffman_node<char>(val, -1);

				//��ǰ�ڵ���
				int dat_old= haffman_rmap[index_char(last->value)].first;
				for(int d=depth; d<max_depth; ++d){
					int cur_bit_new= (dat>>(max_depth-d-1))&1;
					int cur_bit_old= (dat_old>>(max_depth-d-1))&1;
					if(cur_bit_new!=cur_bit_old){
						//�¾ɽڵ��ڵ�ǰλ�в���
						if(cur_bit_new==1){
							*tree= new haffman_node<char>(last, _cur);
						}else{
							*tree= new haffman_node<char>(_cur, last);
						}
						break;
					}
					//�¾ɽڵ��ڵ�ǰλ�޲���
					*tree= new haffman_node<char>(nullptr, nullptr);
					if(cur_bit_new==1){
						tree= &(*tree)->_1;
					}else{
						tree= &(*tree)->_0;
					}
				}
			}else{
					//���½��ڵ� �ݹ����
				if((dat>>(max_depth-depth-1))&1){
					add_leaf(dat, val, &(*tree)->_1, depth+1);
				}else{
					add_leaf(dat, val, &(*tree)->_0, depth+1);
				}
			}
		}
	}

	//��䷴���ѯ
	void fill_rmap(haffman_node<char>* tree, int cur_code=0, int depth=0){
		max_depth= max(max_depth, depth);

		if(tree->leaf()){
			haffman_rmap[index_char(tree->value)]= pair<int, int>(cur_code, depth);
			return;
		}
		fill_rmap(tree->_0, cur_code<<1, depth+1);
		fill_rmap(tree->_1, (cur_code<<1)|1, depth+1);
	}

	void encode(int_8* from, bit_ostream& to, size_t length){
		if(tree_root==nullptr) return;
		for(size_t i=0; i<length; ++i){
			auto temp= haffman_rmap[index_char(from[i])];
			to.push_bit(temp.first, temp.second);
		}
	}
	void decode(bit_istream& from, int_8* to, size_t length){
		if(tree_root==nullptr) return;
		for(size_t i=0; i<length; ++i){
			haffman_node<char>* p= tree_root;
			while(!p->leaf()){
				int temp= from.pop_bit(1);
				switch(temp&1){
					case 0:
						p= p->_0;
						break;
					case 1:
						p= p->_1;
						break;
				}
			}
			to[i]= p->value;
		}
	}
	void show_tree(haffman_node<char>* tree, int depth=0){
		if(tree->leaf()){
			cout<<(int)tree->value<<endl;return;
		}
		cout<<"0>";show_tree(tree->_0, depth+1);
		for(int i=0;i<depth;++i) cout<<"  ";
		cout<<"1>";show_tree(tree->_1, depth+1);
	}
	haffman_node<char>* tree_root;
	pair<int, int> haffman_rmap[256];
	int max_depth;
};

}
#endif