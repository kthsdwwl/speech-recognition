#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include "define.h"
#include "train.h"
#include "min_edit_dist.h"
using namespace std;

namespace medw {

/**
 * global variables
 */
double dp[max_x][max_y];
bool valid[max_x][max_y];
int arrow[max_x][max_y];       // 0 means horizontal arrow, 1 and 2 means diagonal arrow
int temp_num;                  // store the actual number of templates
int range[max_temp_num][2];
double templates[max_temp_num][max_x][feat_num];
double inputstr[max_x][feat_num];
int input_len;
train::Model* models[max_model_num];

/**
 * initialize template
 */
void addTemplate(const char* filename, const int &index)
{
	int win_size;
	FILE* file = fopen(filename, "r");

	fscanf(file, "%d", &win_size);
	for (int j = 0; j < feat_num; ++j) {
		templates[index][0][j] = 0;
		for (int i = 1; i <= win_size; ++i)
			fscanf(file, "%lf", &templates[index][i][j]);
	}
	// range in the dp array
	if (index == 0)
		range[index][0] = 0; 
	else
		range[index][0] = range[index - 1][1] + 1;
	range[index][1] = range[index][0] + win_size; 
	temp_num = index + 1; // how many templates

	fclose(file);
}

/**
 * initialize model
 */
void addModel(train::Model* model, const int &index)
{
	models[index] = model;
	// range in the dp array
	if (index == 0)
		range[index][0] = 0; 
	else
		range[index][0] = range[index - 1][1] + 1;
	range[index][1] = range[index][0] + state_num - 1; 
	temp_num = index + 1;
}

void addModel(const char* filename, const int &index)
{
	if (TYPE == 1)
		models[index] = new train::SGModel();
	else if (TYPE == 2)
		models[index] = new train::GMModel();

	models[index]->readFromFile(filename);

	// range in the dp array
	if (index == 0)
		range[index][0] = 0; 
	else
		range[index][0] = range[index - 1][1] + 1;
	range[index][1] = range[index][0] + state_num - 1; 
	/*if (index==10)
		range[index][1] = range[index][0] + 3 - 1;*/
	temp_num = index + 1;
}

/**
 * initialize input
 */
void initInput(const char* filename)
{
	int win_size;
	FILE* file = fopen(filename, "r");

	fscanf(file, "%d", &win_size);
	for (int j = 0; j < feat_num; ++j) {
		inputstr[0][j] = 0;
		for (int i = 1; i <= win_size; ++i)
			fscanf(file, "%lf", &inputstr[i][j]);
	}
	input_len = win_size;

	fclose(file);
}

/**
 * initialize input
 */
void initInput(double sequence[][feat_num], const int &len)
{
	for (int i = 0; i <= len; ++i)
		for (int j = 0; j < feat_num; ++j)
			inputstr[i][j] = sequence[i][j];
	input_len = len;
}

void outputEachTrellis(const int &k)
{
	char empty = ' ';
	for (int j = range[k][1]; j >= range[k][0]; --j) {
		for (int i = 0; i <= input_len; ++i) {
			if (valid[i][j])
				printf("%9.2lf\t", dp[i][j]);
			else
				printf("%9c\t", empty);
		}
		printf("\n");
	}

	for (int i = 0; i <= input_len; ++i)
		printf("%9d\t", i);
	printf("\n");
}

/**
 * update the segmentation of k th template of a model
 * param index: the model that need to be updated
 */
void updateModelSegment(const int &index, const int &k, const int &k_len)
{
	models[index]->updateSegment(k, k_len, arrow);
}

/**
 * calculate node and edge cost
 * param i: i th frame of the input string
 * param k: k th template
 * param j: j th frame of the k th template
 */
double nodeAndEdgeCost(const int &i, const int &k, const int &j, const int &pre_j)
{
	double edge_cost = 0, node_cost = 0;
	if (TYPE == 0) { // simple DTW
		double sum = 0;
		for (int n = 0; n < feat_num; ++n)
			sum += (inputstr[i][n] - templates[k][j][n]) * (inputstr[i][n] - templates[k][j][n]);
		node_cost = sqrt(sum);
	}
	else { // single Gaussian HMM
		edge_cost = models[k]->trans[pre_j][j];
		node_cost = models[k]->calcNodeCost(inputstr[i], j);
	}
	return node_cost + edge_cost;
}

/**
 * calculate min edit distance 
 */
MatchData minEditDist()
{
	int i, j, k;
	int next_min_cost = 0; // min cost of the next column.
	int cur_min_cost = 0;  // min cost of current column
	MatchData match_data(inf, -1);
	memset(valid, false, sizeof(bool) * (input_len+1) * max_y);

	// initialize dp
	for (k = 0; k < temp_num; ++k)
	{		
		dp[0][range[k][0]] = 0;
		valid[0][range[k][0]] = 1;
	}

	for (i = 0; i < input_len; ++i) {
		cur_min_cost = next_min_cost; next_min_cost = inf; 
		for (k = 0; k < temp_num; ++k){ // for each template string
			for (int j = range[k][0]; j <= range[k][1]; ++j)
			{
				if (valid[i][j])
				{
					if (dp[i][j] > cur_min_cost + beam)
					{
						valid[i][j]=false;
						continue;
					}
					
					// horizontal
					double cost = nodeAndEdgeCost(i + 1, k, j - range[k][0], j - range[k][0]);
					if (!valid[i + 1][j] || dp[i][j] + cost < dp[i + 1][j]) {
						dp[i + 1][j] = dp[i][j] + cost; 
						arrow[i + 1][j] = 0;
					}
					if (dp[i + 1][j] < next_min_cost)
						next_min_cost = dp[i + 1][j];
					valid[i + 1][j] = true;
					
					if (j != range[k][1])
					{
						// diagonal_1
						cost = nodeAndEdgeCost(i + 1, k, j + 1 - range[k][0], j - range[k][0]);
						if (!valid[i + 1][j + 1] || dp[i][j] + cost < dp[i + 1][j + 1]) {
							dp[i + 1][j + 1] = dp[i][j] + cost;
							arrow[i + 1][j + 1] = 1;
						}
						if (dp[i + 1][j + 1] < next_min_cost)
							next_min_cost = dp[i + 1][j + 1];
						valid[i + 1][j + 1] = true;

						// diagonal_2
						if (j != range[k][1] - 1) {
							cost = nodeAndEdgeCost(i + 1, k, j + 2 - range[k][0], j - range[k][0]);
							if (!valid[i + 1][j + 2] || dp[i][j] + cost < dp[i + 1][j + 2]) {
								dp[i + 1][j + 2] = dp[i][j] + cost;
								arrow[i + 1][j + 2] = 2;
							}
							if (dp[i + 1][j + 2] < next_min_cost)
								next_min_cost = dp[i + 1][j + 2];
							valid[i + 1][j + 2] = true;
						}
					}

				}
			}
		}
	}

	for (k = 0; k < temp_num; ++k){
		if (valid[input_len][range[k][1]])
		{
			if (dp[input_len][range[k][1]] < match_data.cost )
			match_data.update(dp[input_len][range[k][1]], k);
		}
	}
	
	return match_data;
}

}