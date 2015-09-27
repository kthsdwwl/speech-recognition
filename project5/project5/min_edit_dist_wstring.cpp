#include <stdio.h>
#include <memory.h>
#include <algorithm>
#include <string.h>
#include "min_edit_dist_wstring.h"
#include "define.h"
using namespace std;

namespace medw {

// global variables
int dp[max_x][max_y];
int arrow[max_x][max_y]; // 0 means horizontal arrow, 1 means vertical arrow, 2 means diagonal arrow
int temp_num; // store the actual number of templates
int templength[max_temp_num]; // store length of each temp
int range[max_temp_num][2];
bool abandon[max_temp_num]; // whether to abandon a template 
char templates[max_x][max_word_len];
char inputstr[max_x][max_word_len];
int inputlen;

/**
 * initialize
 */
void initTemplates(char template_string[][max_word_len], const int& size)
{
	for (int i = 0; i < size; ++i) 
		strcpy(templates[i], template_string[i]);
	temp_num = 1;
	
	range[0][0] = 0; range[0][1] = size;
	templength[0] = size;
}

void initInput(char input_string[][max_word_len], const int& size)
{
	inputlen = size;
	for (int i = 0; i < size; ++i) 
		strcpy(inputstr[i], input_string[i]);
}

void init()
{
	//memset(dp, -1, sizeof(dp));
	memset(abandon, 0, sizeof(abandon));
}

void backTrace(const int &k, MatchData &data)
{
	int i = inputlen, j = range[k][1];
	while (i != 0 || j != range[k][0]) {
		if (arrow[i][j] == 0) { --i; ++data.cost_ins; }

		else if (arrow[i][j] == 1) { --j; ++data.cost_del; }

		else { --i; --j; }
	}
	data.cost_rep = data.cost - data.cost_ins - data.cost_del;
}

/**
 * update next column of the dp array according to current value dp[i][j]
 * at the same time update the min cost of the next column.
 */
void updateNextCol(const int &i, const int &j, const int &k, int &next_min_cost, int flag)
{
	if (flag) {
		if (j == range[k][0]) { dp[i + 1][j] = -1; }
		if (j < range[k][1]) {
			if (dp[i + 1][j] != -1) { dp[i + 1][j + 1] = dp[i + 1][j] + 1; arrow[i + 1][j + 1] = 1; }
			else { dp[i + 1][j + 1] = -1; }
		}
		return;
	}

	// update the entry on the right
	if (j == range[k][0] || dp[i + 1][j] == -1 || dp[i][j] + 1 < dp[i + 1][j]) {
		dp[i + 1][j] = dp[i][j] + 1;
		arrow[i + 1][j] = 0;
		if (dp[i + 1][j] < next_min_cost)
			next_min_cost = dp[i + 1][j];
	}
	// update the entry on upper right
	if (j < range[k][1]) {
		int temp = dp[i][j] + (strcmp(templates[j], inputstr[i]) == 0 ? 0 : 1);
	
		if (temp < dp[i + 1][j] + 1) {
			dp[i + 1][j + 1] = temp; arrow[i + 1][j + 1] = 2;
		}
		else {
			dp[i + 1][j + 1] = dp[i + 1][j] + 1; arrow[i + 1][j + 1] = 1;
		}
		if (dp[i + 1][j + 1] < next_min_cost)
			next_min_cost = dp[i + 1][j + 1];
	}
}

/** 
 * update column i for template k, param cost is the smallest cost of this column. 
 * at the same time update min_cost of the next column
 */
void updateColForTempK(const int &i, const int &k, const int &cur_min_cost, int &next_min_cost, MatchData &match_data)
{
	bool abandon_k = true;
	for (int j = range[k][0]; j <= range[k][1]; ++j) {
		if (i == 0)
			dp[i][j] = j - range[k][0];
		if (dp[i][j] != -1) {
			if (dp[i][j] > cur_min_cost + beam) {
				dp[i][j] = -1;
				updateNextCol(i, j, k, next_min_cost, 1);
			}
			else {
				abandon_k = false;
				updateNextCol(i, j, k, next_min_cost);
			}
		}
		else { // if dp[i][j] is -1, update next column in a different way
			updateNextCol(i, j, k, next_min_cost, 1);
		}
	}
	abandon[k] = abandon_k;

	// update min edit dist among all the templates as well as the index of the closest template
	if (i == inputlen - 1 && dp[i + 1][range[k][1]] != -1 && dp[i + 1][range[k][1]] < match_data.cost)
		match_data.update(dp[i + 1][range[k][1]]);
}

/**
 * calculate min edit distance with beam search pruning
 */
MatchData minEditDistPrune(const int &type)
{
	init();
	int next_min_cost = 0; // min cost of the next column.
	int cur_min_cost = 0;
	MatchData match_data(9999, 0, 0, 0);

	for (int i = 0; i < inputlen; ++i) {
		cur_min_cost = next_min_cost; next_min_cost = 9999; // get the min cost of this column
		if (type == 1) cur_min_cost = 0;
		for (int k = 0; k < temp_num; ++k){ // for each template string
			if (abandon[k]) continue;
			updateColForTempK(i, k, cur_min_cost, next_min_cost, match_data);
		}
	}
	return match_data;
}

}