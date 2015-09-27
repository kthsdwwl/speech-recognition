#include <stdio.h>
#include <memory.h>
#include <algorithm>
#include <math.h>
#include <string.h>
#include "min_edit_dist.h"
#include "define.h"
#include "wav2feat.h"
#include "fsgmodel.h"
using namespace std;

namespace med {

char word_table[15][5] = {"0", "1", "2", "3", "4", "5",
						 "6", "7", "8", "9", "0", 
						 "11", "12", "13", "14"};

// global variables
bool valid[max_x][max_state_num];
bool tmp_valid[max_x][max_state_num];
double dp[max_x][max_state_num];
double inputstr[max_x][feat_num];
int input_len;

// back pointer table
int back_pointer[max_x][max_state_num]; 
int pre_node[max_bpt_node_num];
int node_value[max_bpt_node_num];
bool visited[max_x][max_fsg_state_num];
int node_num;
int initial_node;

// model
FSGModel *model;
int state_n;

/**
 * initialize
 */

void initInput(char *filename)
{
	wav2feat(filename, inputstr, input_len);
}

void initModel(const char *modelpath)
{
	model = new FSGModel();
	model->readFile(modelpath);
	model->drawGraph();
	state_n = model->end_state + 1;
}

void delModel()
{
	delete model;
}

void outputTrellis()
{
	FILE *fp = fopen("dp.txt", "w");
	FILE *fp2 = fopen("bpt.txt", "w");
	char empty = ' ';
	for (int j = 347; j >= 0; --j) {
		for (int i = 0; i <= input_len; ++i) {
			if (valid[i][j]) {
				fprintf(fp, "%9.2lf\t", dp[i][j]);
				fprintf(fp2, "%9d\t", back_pointer[i][j]);	
			}
			else {
				fprintf(fp, "%9c\t", empty);
				fprintf(fp2, "%9c\t", empty);
			}
		}
		fprintf(fp, "\n");
		fprintf(fp2, "\n");
	}

	fclose(fp);
	fclose(fp2);
}

void writeResult(FILE *fp)
{
	int node = initial_node, len = 0;
	char seq[50][5];
	while (node != 0) {
		if (node_value[node] != -1 && node_value[node] != sil_id)
			strcpy(seq[len++], word_table[node_value[node]]);
		node = pre_node[node];
	}
	for (int i = len-1; i >= 0; --i) {
		//printf("%s ", seq[i]);
		fprintf(fp, "%s", seq[i]);
	}
	//printf("\n");
	fprintf(fp, "\n");
}

/**
 * calculate min edit distance with beam search pruning
 */
double minEditDist()
{
	int i, j, k;
	double next_min_cost = 0; // min cost of the next column.
	double cur_min_cost = 0;  // min cost of current column
	bool flag = false; // one column should be traversed again
	node_num = 1;

	//printf("%d %d\n", state_n, input_len);

	memset(valid, false, sizeof(bool) * (input_len+1) * max_state_num);
	memset(tmp_valid, true, sizeof(bool) * (input_len+1) * max_state_num);
	memset(visited, false, sizeof(bool) * (input_len+1) * 10);
	
	// initialize dp
	valid[0][0] = true;
	dp[0][0] = 0;
	back_pointer[0][0] = 0;

	for (i = 0; i <= input_len; ++i) {
		cur_min_cost = next_min_cost; next_min_cost = INF; 
		flag = false;
		for (j = 0; j < state_n; ++j)
		{
			if (!valid[i][j] || !tmp_valid[i][j]) continue;

			if (dp[i][j] > cur_min_cost + beam)
			{
				valid[i][j]=false;
				continue;
			}

			int son_n = model->son_num_of_state[j];
			for (k = 0; k < son_n; ++k)
			{
				int son_id = model->son_of_state[j][k];
				if (model->is_emit[son_id] < 0) { // non emitting state
					if (son_id < j) flag = true; // loop back

					if (!valid[i][son_id] || (valid[i][son_id] && dp[i][j] + model->trans[j][k] < dp[i][son_id])) {
						dp[i][son_id] = dp[i][j] + model->trans[j][k];
						valid[i][son_id] = tmp_valid[i][son_id] = true; // activate

						if (!visited[i][model->fsg_state_index[son_id]]) { // create new bpt node
							visited[i][model->fsg_state_index[son_id]] = true;
							back_pointer[i][son_id] = node_num;
							++node_num;
						}
						pre_node[back_pointer[i][son_id]] = back_pointer[i][j];
						node_value[back_pointer[i][son_id]] = model->is_emit[j];
					}
				}
				else { // emitting state
					if (i == input_len) continue;
					double node_cost = model->calcNodeCost(inputstr[i+1],son_id);
					if (!valid[i+1][son_id] || 
						(valid[i+1][son_id] && 
						dp[i][j] + model->trans[j][k] + node_cost < dp[i+1][son_id])) {
								
							dp[i+1][son_id] = dp[i][j] + model->trans[j][k] + node_cost;
							valid[i+1][son_id] = true;
							back_pointer[i+1][son_id] = back_pointer[i][j];

							if (dp[i+1][son_id] < next_min_cost)
								next_min_cost = dp[i+1][son_id];
					}
				}
			}					
			 
		}
		if (flag) {
			//printf("hehehehehe %d\n", i);
			memset(tmp_valid[i], false, sizeof(bool)*state_n);
			tmp_valid[i][0] = true;
			--i;
		}
	}

	initial_node = back_pointer[input_len][state_n-1];
	return dp[input_len][state_n-1];
}

}