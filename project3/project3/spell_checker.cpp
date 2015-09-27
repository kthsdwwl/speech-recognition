#include <stdio.h>
#include <time.h>
#include <vector>
#include "min_edit_dist.h"
#include "min_edit_dist_wstring.h"
using namespace std;

const char* DFILENAME = "dict.txt";
const char* SFILENAME = "story.txt";
const char* MSFILENAME = "mystory.txt";
const char* CSFILENAME = "storycorrect.txt";
const int max_word_num = 650;
const int max_word_len = 50;
const int type = 1; // 0 is beam search

char story[max_word_num][max_word_len];
char correct_story[max_word_num][max_word_len];

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
	while (fscanf(file, "%s", temp) == 1) { med::initTemplates(temp, i); ++i; }

	fclose(file);
	return i;
}

void spellCheckAndWriteFile(const int& input_num)
{
	FILE* file;
	file = fopen(MSFILENAME, "w");
	for (int i = 0; i < input_num; ++i) {
		med::initInput(story[i]);
		med::MatchData data = med::minEditDistPrune(type);
		fprintf(file, "%s\n", data.best_match);
	}
	fclose(file);
}

void doSpellCheck()
{
	printf("=== Doing spell check ===\n");
	int word_num = readStory(SFILENAME, story);
	readTemplates(DFILENAME);
	spellCheckAndWriteFile(word_num);
	printf("=== Done ===\n");
}

void doErrorCount(const char* file1, const char* file2)
{
	char story1[max_word_num][max_word_len];
	char story2[max_word_num][max_word_len];
	int num1 = readStory(file1, story1);
	int num2 = readStory(file2, story2);
	medw::initTemplates(story1, num1);
	medw::initInput(story2, num2);
	medw::MatchData data = medw::minEditDistPrune();
	medw::backTrace(0, data);

	printf("=== Error count ===\nFile1: %s\nFile2: %s\n", file1, file2);
	printf("Total errors: %d \nInsert: %d\nDelete: %d\nReplace: %d\n\n", data.cost, data.cost_ins, data.cost_del, data.cost_rep);
}

void doSimpleWordCheck()
{
	const int n = 5;
	char* templates[n] = {"elephant", "elegant", "hello", "verylonglongword", "sycophant"};
	char* input = "elegant";
	for (int i = 0; i < n; ++i)
		med::initTemplates(templates[i], i);
	med::initInput(input);
	med::MatchData data = med::minEditDistPrune(type);

	printf("Input: %s\n", input);
	printf("Best match: %s\n", data.best_match);
	med::outputTrellisAndBestPath(data);
}

int main()
{
	clock_t start, end;
	start = clock();
	
	//doSimpleWordCheck();
	doSpellCheck();
	doErrorCount(MSFILENAME, CSFILENAME);
	//doErrorCount(CSFILENAME, SFILENAME);

	end = clock();
	printf("total time: %.8f\n", (double)(end - start)/CLOCKS_PER_SEC);
	return 0;
}