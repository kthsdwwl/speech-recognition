#ifndef _MIN_EIDT_DIST_W_H_
#define _MIN_EIDT_DIST_W_H_

#include <vector>
#include <string.h>

namespace medw {

typedef struct MatchData{
	int cost, cost_rep, cost_ins, cost_del;

	MatchData(const int &_cost, const int &_cost_rep, const int &_cost_ins, const int &_cost_del):
			 cost(_cost),
		     cost_rep(_cost_rep),
			 cost_ins(_cost_ins),
			 cost_del(_cost_del)
	{}

	void update(const int &_cost)
	{
		cost = _cost; 
	}
} MatchData;

/**
 * initialize functions
 */
void init();

void initTemplates(char template_string[][30], const int& size);

void initInput(char input_string[][30], const int& size);
/**
 * backtrace
 */
void backTrace(const int &k, MatchData &data);

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