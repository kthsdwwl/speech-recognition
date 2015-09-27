//#include <memory.h>
//#include <stdio.h>
//#include "lang_model.h"
//#include "lex_tree.h"
//
//void LangModel::init(const char *path)
//{
//	total_number = 0;
//	memset(word_freq, 0, sizeof(word_freq));
//	root = lex::createTree(dict_path, lex_path);
//
//	FILE *file = fopen(path, "r");
//	char word[max_word_len];
//	while (fscanf(file, "%s", word) == 1) {
//		dealWithWord(word);
//		int index = lex::search(root, word);
//		if (word_freq[index] == 0)
//			++total_number;
//		++word_freq[index];
//	}
//	fclose(file);
//}
//
//double LangModel::getWordFreq(const char *word)
//{
//	int index = lex::search(root, word);
//	return (double)(word_freq[index]) / total_number;
//}
//
//void LangModel::dealWithWord(char *word)
//{
//	char* p = word;
//	char* q = word;
//	while (*q) {
//		if ((*q) >= 'A' && (*q) <= 'Z') 
//			*(p++) = (*q) | 0x20;
//		else if ((*q) >= 'a' && (*q) <= 'z')
//			*(p++) = *q;
//		++q;
//	}
//	*p = '\0';	
//}