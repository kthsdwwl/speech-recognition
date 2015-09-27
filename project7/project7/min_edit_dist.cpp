#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include "define.h"
#include "train.h"
#include "model.h"
#include "min_edit_dist.h"
using namespace std;

namespace medw {
/**
 * global variables
 */
double dp[max_x][max_sen_state_num];
bool valid[max_x][max_sen_state_num];
int arrow[max_x][max_sen_state_num];       // 0 means horizontal arrow, 1 and 2 means diagonal arrow

char file_name[max_sen_num][30];
double input[max_sen_num][max_x][feat_num]; // input audio for trainin
int input_len[max_sen_num];                 // input length
int template_state_num[max_sen_num];
bool has_word[max_sen_num][max_train_words] = {false};
int word_of_row[max_sen_num][max_sen_state_num]; // translate row num into word
int state_of_row[max_sen_num][max_sen_state_num];// translate row num into state
int word_in_sen[max_train_words][max_sen_num];   // how many times a word appears in a sentence
int seg_range[max_train_words][max_sen_num][max_state_size][max_sen_words*2];

void initparam()
{
	memset(word_in_sen, 0, sizeof(word_in_sen));
}

void dealWithFilename(int &index, char *file_name)
{
	char* p = file_name;
	char* q = file_name;
	int row = 0, i;
	has_word[index][11] = true;
	for (i = 0; i < silence_state_num; ++i)
	{
		word_of_row[index][i] = 11;
		state_of_row[index][i] = i;
	}
	while (*q) {
		if (*q >= '0' && *q <= '9')
		{
			has_word[index][*q-'0'] = true;
			for (int j = 0; j < state_num-1; ++j)
			{
				word_of_row[index][i+j] = *q - '0';
				state_of_row[index][i+j] = j+1;
			}
			i += state_num-1;

		}
		if (*q == 'Z') 
		{
			has_word[index][0] = true;
			for (int j = 0; j < state_num-1; ++j)
			{
				word_of_row[index][i+j] = 0;
				state_of_row[index][i+j] = j+1;
			}
			i += state_num-1;

		}
		if (*q == 'O') 
		{
			has_word[index][10] = true;
			for (int j = 0; j < state_num-1; ++j)
			{
				word_of_row[index][i+j] = 10;
				state_of_row[index][i+j] = j+1;
			}
			i += state_num-1;

		}
		++q;
	}
	for (int j = 0; j < silence_state_num-1; ++j)
	{
		word_of_row[index][i+j] = 11;
		state_of_row[index][i+j] = j+1;
	}
	i += silence_state_num-1;
	template_state_num[index] = i;
}

/**
 * read all templates for the specific word
 */
int initTemplatesForSen(const char *filelist, const char *train_path)
{
	int sen_num = 0;

	FILE* pFile;
	pFile = fopen(filelist , "r");
	if (pFile == NULL) printf("Error opening file\n");
	while (fscanf(pFile, "%s", file_name[sen_num++]) == 1);
	fclose(pFile);
	if (strcmp(file_name[sen_num-1], "") == 0) --sen_num;
	//printf("%d\n", sen_num);

	for (int k = 0; k < sen_num; ++k) {
		char file_path[40];
		dealWithFilename(k, file_name[k]);
		strcpy(file_path, train_path);
		strcat(file_path, file_name[k]);
		//printf("Now reading file %s\n", file_path);
		FILE* file = fopen(file_path, "r");
		int win_size;
		fscanf(file, "%d", &win_size);
		input_len[k] = win_size;
		for (int j = 0; j < feat_num; ++j) {
			input[k][0][j] = 0;
			for (int i = 1; i <= win_size; ++i) {
				fscanf(file, "%lf", &input[k][i][j]);
			}
		}

		fclose(file);
	}
	return sen_num;
}

void outputEachTrellis(const int &index)
{
	FILE* fp = fopen("trellis", "w");
	char empty = ' ';
	for (int j = template_state_num[index]-1; j >= 0; --j) {
		fprintf(fp, "%d--%d\t", word_of_row[index][j], state_of_row[index][j]);
		for (int i = 0; i <= input_len[index]; ++i) {
			if (valid[i][j])
				fprintf(fp, "%9.2lf\t", dp[i][j]);
			else
				fprintf(fp, "%9c\t", empty);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

/**
 * update the segmentation of k th template of a model
 * param index: the model that need to be updated
 */
void updateSegmentation(GMModel* models[], const int &index)
{
	int word, state, times;

	int i = input_len[index], j = template_state_num[index]- 1; // j means current state
	//printf("index: %d, length: %d\n", index, i);
	seg_range[word_of_row[index][j]][index][state_of_row[index][j]][1] = i;
	while (i > 0 || j > 0)
	{
		word = word_of_row[index][j]; state = state_of_row[index][j]; times = word_in_sen[word][index];
		if (arrow[i][j] == 0)
			--i;
		else if (arrow[i][j] == 1)
		{
			seg_range[word][index][state][times*2] = i;
			models[word]->seg_size[state] += seg_range[word][index][state][times*2+1] - seg_range[word][index][state][times*2] + 1;
			
			--i;
			--j;
			
			int new_word = word_of_row[index][j], new_state = state_of_row[index][j];
			if (new_word != word || state < new_state) // when it jumps to a new word, increase the time old word appears in sentence(index)
				++word_in_sen[word][index]; 
			int new_times = word_in_sen[new_word][index];
			seg_range[new_word][index][new_state][new_times*2+1] = i;
		}
		else if (arrow[i][j] == 2)
		{
			seg_range[word][index][state][times*2] = i;
			models[word]->seg_size[state] += seg_range[word][index][state][times*2+1] - seg_range[word][index][state][times*2] + 1;
			seg_range[word][index][state-1][times*2] = -1; // since one state is skipped, so the range is set to be -1
			//if (word == 2 && state == 4)
			//	printf("heiheiheihei\n %d\nheiheihei\n", models[word]->seg_size[state]);
			--i;
			j-=2;
			
			int new_word = word_of_row[index][j], new_state = state_of_row[index][j];
			if (new_word != word || state < new_state)
				++word_in_sen[word][index];
			int new_times = word_in_sen[new_word][index];
			seg_range[new_word][index][new_state][new_times*2+1] = i;
		}
	}
	
	// at this moment i = 0 and j = 0, which is dummy state
	++word_in_sen[11][index];
	/*models[word_of_row[index][j]]->seg_range[index][state_of_row[index][j]][0] = i;
	models[word_of_row[index][j]]->seg_size[state_of_row[index][j]] += models[word_of_row[index][j]]->seg_range[index][state_of_row[index][j]][1] - models[word_of_row[index][j]]->seg_range[index][state_of_row[index][j]][0] + 1;*/
}

void updateMeanCovTrans(int sen_num, GMModel* models[])
{
	for (int i = 0; i < max_train_words; ++i)
	{
		//printf("Updating word %d\n", i);
		models[i]->updateMeansAndCov(input, seg_range[i], word_in_sen[i], sen_num);
		models[i]->updateTrans(input, seg_range[i], word_in_sen[i], sen_num);
	}
}

/**
 * calculate node and edge cost
 * param i: i th frame of the input string
 * param k: k th template
 * param j: j th frame of the k th template
 */
double nodeAndEdgeCost(GMModel* models[], const int &index, const int &i, const int &j, const int &pre_j)
{
	double edge_cost = 0, node_cost = 0;
	int pre_word = word_of_row[index][pre_j], word = word_of_row[index][j];
	int pre_state = state_of_row[index][pre_j], state = state_of_row[index][j];
	if (word != pre_word || pre_state > state)
		edge_cost = models[word]->trans[0][state];
	else
		edge_cost = models[word]->trans[pre_state][state];
	node_cost = models[word]->calcNodeCost(input[index][i], state);

	return node_cost + edge_cost;
}

/**
 * calculate min edit distance 
 */
double minEditDist(int index, GMModel* models[])
{
	int i, j;
	int next_min_cost = 0; // min cost of the next column.
	int cur_min_cost = 0;  // min cost of current column
	//MatchData match_data(INF, -1);
	int len = input_len[index];
	memset(valid, false, sizeof(bool) * (len+1) * max_sen_state_num);

	// initialize dp
	dp[0][0] = 0;
	valid[0][0] = 1;

	for (i = 0; i < len; ++i) {
		cur_min_cost = next_min_cost; next_min_cost = INF; 
		for (j = 0; j < template_state_num[index]; ++j)
		{
			if (!valid[i][j]) continue;

			if (dp[i][j] > cur_min_cost + beam)
			{
				valid[i][j]=false;
				continue;
			}
				
			// horizontal
			double cost = nodeAndEdgeCost(models, index, i + 1, j, j);
			if (!valid[i + 1][j] || dp[i][j] + cost < dp[i + 1][j]) {
				dp[i + 1][j] = dp[i][j] + cost; 
				arrow[i + 1][j] = 0;
			}
			if (dp[i + 1][j] < next_min_cost)
				next_min_cost = dp[i + 1][j];
			valid[i + 1][j] = true;
				
			// diagonal_1
			if (j < template_state_num[index]-1)
			{
				cost = nodeAndEdgeCost(models,index, i+1, j + 1, j);
				if (!valid[i + 1][j + 1] || dp[i][j] + cost < dp[i + 1][j + 1]) {
					dp[i + 1][j + 1] = dp[i][j] + cost;
					arrow[i + 1][j + 1] = 1;
				}
				if (dp[i + 1][j + 1] < next_min_cost)
					next_min_cost = dp[i + 1][j + 1];
				valid[i + 1][j + 1] = true;

				// diagonal_2
				if ((word_of_row[index][j] == 11 && state_of_row[index][j] != silence_state_num - 2) || 
					(word_of_row[index][j] < 11 && state_of_row[index][j] != state_num - 2)) {
					cost = nodeAndEdgeCost(models, index, i + 1, j + 2, j);
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

	if (valid[len][template_state_num[index]-1]) 
		return dp[len][template_state_num[index]-1];

	return double(INF);
}

}