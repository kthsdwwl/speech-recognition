#include <stdio.h>
#include <memory.h>
#include <algorithm>
#include <math.h>
#include <string.h>
#include "min_edit_dist.h"
using namespace std;

namespace med {

// parameters
const int beam = 3;
const int max_x = 50;
const int max_y = 100000;
const int max_temp_num = 7000;
const int threshold = 3;

// global variables
int dp[max_x][max_y];
int arrow[max_x][max_y]; // 0 means horizontal arrow, 1 means vertical arrow, 2 means diagonal arrow
int temp_num; // store the actual number of templates
int templength[max_temp_num]; // store length of each temp
int range[max_temp_num][2];
bool abandon[max_temp_num]; // whether to abandon a template 
char templates[max_temp_num][max_x];
char inputstr[max_x];
char curtempstr[max_x];

/**
 * initialize
 */

void initTemplates(const char* template_string, const int &index)
{
	strcpy(templates[index], template_string);
	temp_num = index + 1;
}

void initInput(const char* input_string)
{
	strcpy(inputstr, input_string);
}

void init()
{
	//memset(dp, -1, sizeof(dp));
	memset(abandon, 0, sizeof(abandon));

	range[0][0] = 0;
	for (int k = 0; k < temp_num; ++k) {
		templength[k] = strlen(templates[k]);
		range[k][1] = range[k][0] + templength[k];
		range[k + 1][0] = range[k][1] + 1;
	}
}

/**
 * output dp trellis of the specific template string and input string
 */
void outputEachTrellis(const int &templen, const int &inputlen, const int &k)
{
	for (int j = range[k][1]; j >= range[k][0]; --j) {
		if (j != range[k][0]) printf("%c\t", templates[k][j - range[k][0] - 1]);
		else printf("0\t");

		for (int i = 0; i <= inputlen; ++i)
			printf("%2d\t", dp[i][j]);
		printf("\n");
	}

	// print input string
	if (k != temp_num - 1)
		return;
	printf(" \t 0\t");
	for (int i = 0; i < inputlen; ++i)
		printf("%2c\t", inputstr[i]);
	printf("\n");
}

/**
 * output best path
 */
void outputBestPath(const int &templen, const int &inputlen, const int &k)
{
	for (int i = 0; i <= inputlen; ++i)
		arrow[i][0] = 0;
	for (int j = range[k][0]; j <= range[k][1]; ++j)
		arrow[0][j] = 1;
	
	int path[max_x], pathlen = 0;
	int i = inputlen, j = range[k][1];
	while (i != 0 || j != range[k][0]) {
		path[pathlen++] = arrow[i][j];
		if (arrow[i][j] == 0) { --i; }

		else if (arrow[i][j] == 1) { --j; }

		else { --i; --j; }
	}

	for (int p = pathlen - 1; p >= 0; --p)
		printf("%d ", path[p]);
	printf("\n\n");
}

/**
 * output data for each template string
 */
void outputTrellisAndBestPath(const MatchData &data)
{
	int inputlen = strlen(inputstr);
	
	printf("=== Trellis ===\n");
	for (int k = 0; k < temp_num; ++k)
	{
		int templen = strlen(templates[k]);
		outputEachTrellis(templen, inputlen, k);
		printf("----------------------------\n");
	}
	printf("=== Best Path ===\n");
	outputBestPath(strlen(data.best_match), inputlen, data.best_index);
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
		int temp = dp[i][j] + (curtempstr[j - range[k][0]] == inputstr[i] ? 0 : 1);
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
	strcpy(curtempstr, templates[k]);
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
	if (i == strlen(inputstr) - 1 && dp[i + 1][range[k][1]] != -1 && dp[i + 1][range[k][1]] < match_data.cost)
		match_data.update(dp[i + 1][range[k][1]], k, templates[k]);
}

/**
 * calculate min edit distance with beam search pruning
 */
MatchData minEditDistPrune(const int &type)
{
	init();
	int inputlen = strlen(inputstr);
	int next_min_cost = 0; // min cost of the next column.
	int cur_min_cost = 0;
	MatchData match_data(9999, 0, 0, 0);

	for (int i = 0; i < inputlen; ++i) {
		cur_min_cost = next_min_cost; next_min_cost = 9999; // get the min cost of this column
		if (type == 1) cur_min_cost = 0;
		for (int k = 0; k < temp_num; ++k){ // for each template string
		    // pruning
			if (abandon[k]) continue;
			if (abs((int)(strlen(templates[k]) - strlen(inputstr))) > threshold ) continue;
			if (strcmp(templates[k], inputstr) == 0) {
				match_data.update(0, k, templates[k]);
				return match_data;
			}
			
			updateColForTempK(i, k, cur_min_cost, next_min_cost, match_data);
		}
	}
	return match_data;
}

}