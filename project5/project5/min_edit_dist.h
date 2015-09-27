#ifndef _MIN_EIDT_DIST_H_
#define _MIN_EIDT_DIST_H_

#include <string.h>

namespace med {

typedef struct MatchData{
	int cost;
	int best_index;
	char best_match[50];

	MatchData(const int &_cost, const int &_best_index):
			 cost(_cost),
			 best_index(_best_index)
	{ strcpy(best_match, "this_string_has_no_match"); }

	void update(const int &_cost, const int &_best_index, const char* _best_match)
	{
		cost = _cost; best_index = _best_index; strcpy(best_match, _best_match);
	}
} MatchData;


void readLextrees(const char *lex_path);

/**
 * initialize functions
 */
void init();

void initTemplates(const char* template_string, const int &index);

void initInput(const char* input_string);

void initText(const char* line, const int &index);

/**
 * calculate min edit distance
 */
MatchData minEditDistPrune(const int &type = 0);

/**
 * calculate min edit distance with beam search pruning and segmentate the text
 */
void minDistSegmentation(const int &type, const int & line_index);

/**
 * write segmentation results into file
 */
void writeSegResult(const char* seg_result_path);

}
#endif