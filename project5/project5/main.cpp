#include <stdio.h>
#include <time.h>
#include <math.h>
#include "min_edit_dist.h"
#include "min_edit_dist_wstring.h"
#include "define.h"
#include "lex_tree.h"
using namespace std;

char story[max_story_word_num][max_word_len];
char correct_story[max_story_word_num][max_word_len];

bool isLower(char c)
{
	return c >= 'a' && c <= 'z'; 
}

bool isUpper(char c)
{
	return c >= 'A' && c <= 'Z'; 
}

void dealWithWord(char* word)
{
	char* p = word;
	char* q = word;
	while (*q) {
		if (isUpper(*q)) 
			*(p++) = (*q) | 0x20;
		else if (isLower(*q))
			*(p++) = *q;
		++q;
	}
	*p = '\0';
}

int readStory(const char* filename, char story[][max_word_len])
{
	int i = 0;
	FILE* story_file = fopen(filename, "r");
	while (fscanf(story_file, "%s", story[i]) == 1) {
		dealWithWord(story[i++]);
	}
	fclose(story_file);
	return i;
}

int readTemplates(const char* filename)
{
	int i = 0;
	FILE* file = fopen(filename, "r");
	char temp[max_word_len];
	while (fscanf(file, "%s", temp) == 1) { dealWithWord(temp); med::initTemplates(temp, i); ++i; }

	fclose(file);
	return i;
}

void spellCheckAndWriteFile(const int& input_num)
{
	FILE* file;
	file = fopen(MSFILENAME, "w");
	med::readLextrees(LEXPATH);
	for (int i = 0; i < input_num; ++i) {
		med::initInput(story[i]);
		med::MatchData data = med::minEditDistPrune(TYPE);
		fprintf(file, "%s\n", data.best_match);
	}
	fclose(file);
}

void readText(const char* filename)
{
	int i = 0;
	char temp[max_text_len];
	char whole_text[max_text_len] = "";

	FILE* file = fopen(filename, "r");
	while (fgets(temp, max_text_len, file)) { dealWithWord(temp); strcat(whole_text, temp); ++i; }
	fclose(file);
	
	med::initText(whole_text, 0);
}

void doSegmentation()
{
	printf("=== Doing segmentation===\n");
	printf("Reading file %s\n", UNSEGFILENAME);
	readText(UNSEGFILENAME);
	med::readLextrees(LEXPATH);
	readTemplates(DICTPATH);

	printf("Segmenting file %s\n", UNSEGFILENAME);
	med::minDistSegmentation(TYPE,0);

	printf("Writing result to file %s\n", SEGRESULTFILENAME);
	med::writeSegResult(SEGRESULTFILENAME);
	printf("=== Done ===\n\n");

}

void doSpellCheck()
{
	printf("=== Doing spell check ===\n");
	int word_num = readStory(SFILENAME, story);
	readTemplates(DICTPATH);
	spellCheckAndWriteFile(word_num);
	printf("=== Done ===\n\n");
}

void doErrorCount(const char* file1, const char* file2)
{
	char story1[max_story_word_num][max_word_len];
	char story2[max_story_word_num][max_word_len];
	int num1 = readStory(file1, story1);
	int num2 = readStory(file2, story2);
	medw::initTemplates(story1, num1);
	medw::initInput(story2, num2);
	medw::MatchData data = medw::minEditDistPrune();
	medw::backTrace(0, data);

	printf("=== Error count ===\nFile1: %s\nFile2: %s\n", file1, file2);
	//printf("Total errors: %d \nInsert: %d\nDelete: %d\nReplace: %d\n\n", data.cost, data.cost_ins, data.cost_del, data.cost_rep);
	printf("Word number difference: %d\n", abs(data.cost_ins - data.cost_del));
	printf("Mispelled words: %d\n", data.cost_rep);
	printf("Total error: %d\n\n", abs(data.cost_ins - data.cost_del) + data.cost_rep);
}

void lexTreeTest()
{
	lex::createTree(DICTPATHTEST, LEXPATHTEST);
	lex::drawGraph();
}

int main()
{
	//lexTreeTest();
	//doSpellCheck();
	//doErrorCount(MSFILENAME, CSFILENAME);
	doSegmentation();
	doErrorCount(SEGRESULTFILENAME, CSEGFILENAME);
	return 0;
}