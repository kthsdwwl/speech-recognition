#ifndef _MIN_EIDT_DIST_H_
#define _MIN_EIDT_DIST_H_

#include <string.h>

namespace med {

typedef struct MatchData{
	int cost, cost_rep, cost_ins, cost_del;
	int best_index;
	char best_match[50];

	MatchData(const int &_cost, const int &_cost_rep, const int &_cost_ins, const int &_cost_del):
			 cost(_cost),
		     cost_rep(_cost_rep),
			 cost_ins(_cost_ins),
			 cost_del(_cost_del),
			 best_index(0)
	{ strcpy(best_match, "this_string_has_no_match"); }

	void update(const int &_cost, const int &_best_index, const char* _best_match)
	{
		cost = _cost; best_index = _best_index; strcpy(best_match, _best_match);
	}
} MatchData;

/**
 * initialize functions
 */
void init();

void initTemplates(const char* template_string, const int &index);

void initInput(const char* input_string);

/**
 * output dp trellis of the specific template string and input string
 */
void outputEachTrellis(const int &templen, const int &inputlen, const int &tempindex);

/**
 * output best path
 */
void outputBestPath(const int &templen, const int &inputlen, const int &tempindex);

/**
 * output data for each template string
 */
void outputTrellisAndBestPath(const MatchData &data);

/**
 * backtrace
 */
void backTrace(const int &templen, const int &inputlen, const int &k, MatchData &data);

/**
 * update next column of the dp array according to current value dp[i][j]
 * at the same time update the min cost of the next column.
 */
void updateNextCol(const int &i, const int &j, const int &k, int &next_min_cost, int flag = 0);

/** 
 * update column i for template k, param cost is the smallest cost of this column. 
 * at the same time update min_cost of the next column
 */
void updateColForTempK(const int &i, const int &k, const int &cur_min_cost, int &next_min_cost, MatchData &match_data);

/**
 * calculate min edit distance
 */
MatchData minEditDistPrune(const int &type = 0);

}
#endif