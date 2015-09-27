#ifndef _LANG_MODEL_H_
#define _LANG_MODEL_H_

#include <memory.h>
#include <stdio.h>
#include "lex_tree.h"
#include "define.h"

namespace lm {

class LangModel {
public:
	void init(const char *path);
	void initBiGram(const char *path);

	int getWordFreq(const char *word);
	double wordCost(const char *word);
	double wordCost(const char *word1, const char *word2);
private:
	int total_number;
	int word_freq[max_word_num];
	int word_freq_bi[max_word_num][max_word_num];
	lex::Trie_node *root;
	
	void dealWithWord(char* word);
};

void LangModel::init(const char *path)
{
	char word[max_word_len];
	char story[2000][max_word_len];

	total_number = 0;
	memset(word_freq, 0, sizeof(word_freq));
	memset(word_freq_bi, 0, sizeof(word_freq_bi));
	root = lex::createTree(DICTPATH, LEXPATH);

	int n = 0;
	FILE *file = fopen(path, "r");
	while (fscanf(file, "%s", story[n]) == 1) 
		dealWithWord(story[n++]);
	fclose(file);

	for (int i = 0; i < n; ++i) {
		int index = lex::search(root, story[i]);
		if (word_freq[index] == 0)
			++total_number;
		++word_freq[index];
		if (i + 1 < n) {
			int index2 = lex::search(root, story[i+1]);
			++word_freq_bi[index][index2];
		}
	}
}

int LangModel::getWordFreq(const char *word)
{
	int index = lex::search(root, word);
	return word_freq[index];
}

double LangModel::wordCost(const char *word)
{
	return 1.0 - (double)getWordFreq(word) / total_number;
}

double LangModel::wordCost(const char *word1, const char *word2)
{
	int index1 = lex::search(root, word1);
	int index2 = lex::search(root, word2);
	if (word_freq[index1] == 0)
		return 1.0;
	double condition_prob = (double)(word_freq_bi[index1][index2]) / word_freq[index1];
	return 1.0 - condition_prob;
}

void LangModel::dealWithWord(char *word)
{
	char* p = word;
	char* q = word;
	while (*q) {
		if ((*q) >= 'A' && (*q) <= 'Z') 
			*(p++) = (*q) | 0x20;
		else if ((*q) >= 'a' && (*q) <= 'z')
			*(p++) = *q;
		++q;
	}
	*p = '\0';	
}

}

#endif