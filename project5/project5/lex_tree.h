#ifndef _LEX_TREE_H_
#define _LEX_TREE_H_

#include <cstdio>
#include <ctime>
#include <fstream>
#include "define.h"

//#define _DEBUG
namespace lex {

struct Trie_node
{
	bool is_str;
	int son_num;
	int word_index;
	struct Trie_node *next[max_son_num];
};

void insert(Trie_node *root, const char *s, int &index);

int search(Trie_node *root, const char *s);

void del(Trie_node *root);

void dealWithWord(char* word);


void buildTrie(const char* filename, Trie_node* root);

void getLexPara(Trie_node* root, int next_index, int father_row_num, int son_index);

void writeLextrees(const char* Lextrees_path);

void readLextrees(const char* Lextrees_path);

Trie_node* createTree(const char *dict_path, const char* Lextrees_path);

void drawGraph();

}
#endif