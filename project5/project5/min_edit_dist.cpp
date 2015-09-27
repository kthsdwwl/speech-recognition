#include <stdio.h>
#include <memory.h>
#include <algorithm>
#include <math.h>
#include <string.h>
#include "min_edit_dist.h"
#include "lang_model.h"
#include "define.h"
using namespace std;

namespace med {

// global variables
bool valid[max_x][max_node_num];
double dp[max_x][max_node_num];
char templates[max_word_num][max_word_len];
int back_pointer[max_line_len][max_node_num]; 
int node_backtrace[max_node_num];
int loop_node_table[max_line_len];
int loop_node_num;
int min_word_index;
int temp_num; // store the actual number of templates
int line_num;
int input_len;
int templength[max_temp_num]; // store length of each temp
char inputstr[max_x];
char text[max_line_num][max_line_len];

char Lextrees[max_node_num];
int row_num_of_son[max_node_num][max_son_num];
int row_num_of_father[max_node_num];
int row_num_of_word[max_word_num];
int son_num_of_node[max_node_num];
int word_num_of_row[max_node_num];

lm::LangModel model;

int initial_node;
int node_num = 1;
int count = 0;
int word_num = 0;
int match_row_num;

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
	input_len = strlen(inputstr);
}

void initText(const char* line, const int &index)
{
	strcpy(text[index],line);
	line_num = index+1;
}

void writeSegResult(const char* seg_result_path)
{
	int n = 0;
	char word_sequence[400][max_word_len];
	strcpy(word_sequence[n++], templates[word_num_of_row[match_row_num]]);
	//printf("%s\n",templates[word_num_of_row[match_row_num]]);
	
	initial_node = back_pointer[input_len][match_row_num];
	while (initial_node)
	{
		strcpy(word_sequence[n++], templates[loop_node_table[initial_node]]);
		//printf("%s\n",templates[loop_node_table[initial_node]]);
		initial_node = node_backtrace[initial_node];
	}

	FILE* file;
	file = fopen(seg_result_path, "w");
	for (int i = n-1; i >= 0; --i) 
		fprintf(file, "%s\n", word_sequence[i]);
	fclose(file);
}



/**
 * read the Lextrees parameters
 */
void readLextrees(const char *lex_path)
{
	FILE *fp;
	if ((fp = fopen(lex_path, "rb")) == NULL)
	{
		printf("Unable to open file\n");
		return;
	}
	fread(&word_num, sizeof(int), 1, fp);
	fread(&node_num, sizeof(node_num),1, fp);
	fread(Lextrees, sizeof(char), node_num, fp);
	fread(row_num_of_son, sizeof(int), node_num*max_son_num, fp);
	fread(word_num_of_row, sizeof(int), node_num, fp);
	fread(row_num_of_father, sizeof(int), node_num, fp);
	fread(row_num_of_word, sizeof(int), word_num, fp);
	fread(son_num_of_node, sizeof(int), node_num, fp);

	fclose(fp);
}

/**
 * calculate min edit distance with beam search pruning
 */
MatchData minEditDistPrune(const int &type)
{
	int i, j, k;
	double next_min_cost = 0; // min cost of the next column.
	double cur_min_cost = 0;  // min cost of current column
	MatchData match_data(inf, -1);
	memset(valid, false, sizeof(bool) * (input_len+1) * max_node_num);

	// initialize dp
	for (i = 0; i <= input_len; ++i)
	{
		if (i <= beam)
		{
			valid[i][0] = true;
			dp[i][0] = i;
		}
	}
	for (j = 1; j < node_num; ++j)
	{
		valid[0][j] = true;
		dp[0][j] = dp[0][row_num_of_father[j]] + 1;
	}

	for (i = 0; i < input_len; ++i) {
		cur_min_cost = next_min_cost; next_min_cost = inf; 
		for (j = 0; j < node_num; ++j)
		{
			if (valid[i][j])
			{
				if (dp[i][j] > cur_min_cost + beam)
				{
					valid[i][j]=false;
					continue;
				}
				
				if (son_num_of_node[j])
					for (k = 0; k < son_num_of_node[j]; ++k)
					{
						// vertical
						if (!valid[i][row_num_of_son[j][k]] || (valid[i][row_num_of_son[j][k]] && dp[i][j]+1 < dp[i][row_num_of_son[j][k]]))
						{
							dp[i][row_num_of_son[j][k]] = dp[i][j] + 1;
							valid[i][row_num_of_son[j][k]] = true;
						}
						
						// diagonal
						dp[i + 1][row_num_of_son[j][k]] = dp[i][j] + (Lextrees[row_num_of_son[j][k]] != inputstr[i]);
						valid[i + 1][row_num_of_son[j][k]] = true;
						if (dp[i + 1][row_num_of_son[j][k]] < next_min_cost)
							next_min_cost = dp[i + 1][row_num_of_son[j][k]];
							
					}

				// horizontal
				if (!valid[i + 1][j] || dp[i][j] + 1 < dp[i + 1][j]) 
				{
					dp[i + 1][j] = dp[i][j] + 1; 
					valid[i + 1][j] = true;
					if (dp[i + 1][j] < next_min_cost)
						next_min_cost = dp[i + 1][j];
				}	
			}
		}
	}
					
	for (k = 0; k < word_num; ++k){
		if (valid[input_len][row_num_of_word[k]] && dp[input_len][row_num_of_word[k]] < match_data.cost)
			match_data.update(dp[input_len][row_num_of_word[k]], k, templates[k]);
	}

	match_row_num = row_num_of_word[match_data.best_index];
	
	return match_data;
}

/**
 * calculate min edit distance with beam search pruning and segmentate the text
 */
void minDistSegmentation(const int &type, const int &line_index)
{
	int i, j, k, t;
	double next_min_cost = 0; // min cost of the next column.
	double cur_min_cost = 0;  // min cost of current column
	double min_cost = inf;
	bool new_node = false;
	loop_node_num = 1;
	loop_node_table[0] = -1;
	back_pointer[0][0] = 0;
	input_len = strlen(text[line_index]);
	memset(valid, false, sizeof(bool) * (input_len+1) * max_node_num);
	model.init(MODELTRAINPATH);

	// initialize dp
	for (i = 0; i <= input_len; ++i)
	{
		if (i <= beam)
		{
			valid[i][0] = true;
			dp[i][0] = i;
		}
	}
	for (j = 1; j < node_num; ++j)
	{
		valid[0][j] = true;
		dp[0][j] = dp[0][row_num_of_father[j]] + 1;
		back_pointer[0][j] = 0;
	}


	for (i = 0; i < input_len; ++i) {
		cur_min_cost = next_min_cost; next_min_cost = inf; 
		for (j = 0; j < node_num; ++j)
		{
			if (valid[i][j])
			{
				if (dp[i][j] > cur_min_cost + beam)
				{
					valid[i][j]=false;
					continue;
				}
				
				if (son_num_of_node[j])
					for (k = 0; k < son_num_of_node[j]; ++k)
					{
						// vertical
						if (!valid[i][row_num_of_son[j][k]] || (valid[i][row_num_of_son[j][k]] && dp[i][j]+1 < dp[i][row_num_of_son[j][k]]))
						{
							dp[i][row_num_of_son[j][k]] = dp[i][j] + 1;
							valid[i][row_num_of_son[j][k]] = true;
							back_pointer[i][row_num_of_son[j][k]] = back_pointer[i][j];
						}
						
						// diagonal
						dp[i + 1][row_num_of_son[j][k]] = dp[i][j] + (Lextrees[row_num_of_son[j][k]] != text[line_index][i]);
						valid[i + 1][row_num_of_son[j][k]] = true;
						if (dp[i + 1][row_num_of_son[j][k]] < next_min_cost)
							next_min_cost = dp[i + 1][row_num_of_son[j][k]];
						back_pointer[i+1][row_num_of_son[j][k]] = back_pointer[i][j];
					}

				else
				{
					double cost = 0;
					char cur_word[max_word_len];
					strcpy(cur_word, templates[word_num_of_row[j]]);
					if (lang_model_choice == 1) cost += model.wordCost(cur_word);
					if (lang_model_choice == 2) {
					int pre_node = back_pointer[i][j];
					if (pre_node) {
						char pre_word[max_word_len];
						strcpy(pre_word, templates[loop_node_table[pre_node]]);
						cost += model.wordCost(pre_word, cur_word);
					}
					}
					if (!valid[i+1][0] || (valid[i+1][0] && dp[i+1][0] > dp[i][j]+1+cost))
					{
						dp[i+1][0] = dp[i][j]+1+cost;
						if (dp[i + 1][0] < next_min_cost)
							next_min_cost = dp[i + 1][0];
						valid[i+1][0] = true;
						min_word_index = word_num_of_row[j];
						new_node = true;
					}
				}

				// horizontal
				if (!valid[i + 1][j] || dp[i][j] + 1 < dp[i + 1][j]) 
				{
					dp[i + 1][j] = dp[i][j] + 1; 
					valid[i + 1][j] = true;
					if (dp[i + 1][j] < next_min_cost)
						next_min_cost = dp[i + 1][j];
					back_pointer[i+1][j] = back_pointer[i][j];
				}	
			}
		}

		if (new_node)
		{
			back_pointer[i+1][0] = loop_node_num;
			node_backtrace[loop_node_num] = back_pointer[i][row_num_of_word[min_word_index]];
			loop_node_table[loop_node_num] = min_word_index;
			//printf("loop_node_index ;%d loop_node_word:%s \n", loop_node_num, templates[min_word_index]);
			++loop_node_num;
			new_node = false;
			for (k = 0; k < son_num_of_node[0]; ++k)
			{
				t = dp[i+1][0]+(Lextrees[row_num_of_son[0][k]] != text[line_index][i])-1;
				if (!valid[i+1][row_num_of_son[0][k]] || (valid[i+1][row_num_of_son[0][k]] && t < dp[i+1][row_num_of_son[0][k]]))
				{
					dp[i+1][row_num_of_son[0][k]] = t;
					next_min_cost = t; 
					back_pointer[i+1][row_num_of_son[0][k]] = back_pointer[i+1][0];
				}
			}
		}
	}
	
	for (k = 0; k < word_num; ++k)
		if (valid[input_len][row_num_of_word[k]] && dp[input_len][row_num_of_word[k]] < min_cost)
		{
			match_row_num = row_num_of_word[k];
			min_cost = dp[input_len][row_num_of_word[k]];
		}
		//printf("%.10lf\n\n\n",min_cost);
}


}