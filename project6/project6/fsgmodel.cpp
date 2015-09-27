#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#include "model.h"
#include "fsgmodel.h"

char nums[15][5] = {"0", "1", "2", "3", "4", "5",
						 "6", "7", "8", "9", "10", 
						 "11", "12", "13", "14"};

void FSGModel::readFile(const char *filename)
{
	int edge_start, edge_end, edge_id;
	int temp_id = 1;
	int fsg_state[max_fsg_state_num];
	int out_num[max_fsg_state_num] = {0};
	memset(son_num_of_state, 0, sizeof(son_num_of_state));
	memset(trans, 0, sizeof(trans));
	//memset(is_non_emit, 0, sizeof(is_non_emit));

	FILE *fsg_file = fopen(filename, "r");

	// read basic info
	fscanf(fsg_file, "%d %d %d", &fsg_state_num, &start_state, &end_state);
	for (int i = 0; i < fsg_state_num; ++i) { // fsg state
		fscanf(fsg_file, "%d", &fsg_state[i]);
		fsg_state_index[fsg_state[i]] = i;
		is_emit[fsg_state[i]] = -1; // non emitting
	}

	// read each edge
	while (fscanf(fsg_file, "%d %d %d", &edge_start, &edge_end, &edge_id) == 3) {
		++out_num[fsg_state_index[edge_start]];

		if (edge_id >= 0) { // there is a word in the edge
			int state_n;

			// read word model from file
			char modelname[40];
			strcpy(modelname, WORDMODELPATH);
			strcat(strcat(modelname, "model_"), nums[edge_id]);
			GMModel *model = new GMModel();
			model->readFromFile(modelname);
			
			son_of_state[edge_start][son_num_of_state[edge_start]] = temp_id;
			son_of_state[edge_start][son_num_of_state[edge_start]+1] = temp_id+1;
			trans[edge_start][son_num_of_state[edge_start]] = model->trans[0][1]; 
			trans[edge_start][son_num_of_state[edge_start]+1] = model->trans[0][2];
			son_num_of_state[edge_start] += 2;

			if (edge_id == sil_id) state_n = silence_state_num;
			else	state_n = state_num;

			// for each state in word model
			for (int i = 1; i < state_n; ++i, ++temp_id) {
				int state_id = temp_id;
				is_emit[state_id] = edge_id;
				// mean and covariance
				for (int c = 0; c < cluster_num; ++c) {
					weights[state_id][c] = model->weights[i][c];
					for (int f = 0; f < feat_num; ++f) {
						means[state_id][c][f] = model->means[i][c][f];
						covar[state_id][c][f] = model->covar[i][c][f];
					}
				}
				// translation
				if (i == state_n - 1) {
					son_num_of_state[state_id] = 2;
					son_of_state[state_id][0] = state_id;
					son_of_state[state_id][1] = edge_end;
					//trans[state_id][0] = -log(1.0-EXITPROB); trans[state_id][1] = -log(EXITPROB);
					trans[state_id][0] = model->trans[i][i]; trans[state_id][1] = -log(EXITPROB);
				}
				else if (i == state_n - 2) {
					son_num_of_state[state_id] = 2;
					son_of_state[state_id][0] = state_id; son_of_state[state_id][1] = state_id+1;
					trans[state_id][0] = model->trans[i][i]; trans[state_id][1] = model->trans[i][i+1];
				}
				else {
					son_num_of_state[state_id] = 3;
					son_of_state[state_id][0] = state_id; son_of_state[state_id][1] = state_id+1; son_of_state[state_id][2] = state_id+2;
					trans[state_id][0] = model->trans[i][i]; trans[state_id][1] = model->trans[i][i+1]; trans[state_id][2] = model->trans[i][i+2];
				}
						
			}
			//temp_id += state_num;
			if (temp_id == edge_end)
				++temp_id;
			delete model;
		}
		else { // there is no word in the edge
			son_of_state[edge_start][son_num_of_state[edge_start]++] = edge_end;
		}
		
		//++son_num_of_state[edge_start];
	}
	fclose(fsg_file);
	// assign cost to edge start from fsg states
	for (int i = 0; i < fsg_state_num-1; ++i)
	{
		int state_id = fsg_state[i];
		int son_num = son_num_of_state[state_id];
		//printf("%d, son: %d\n", state_id, son_num);
		for (int j = 0; j < son_num; ++j) {
			trans[state_id][j] += -log((double)1/out_num[fsg_state_index[i]]);
		}
	}
	trans[fsg_state[fsg_state_num-1]][0] = LOOPCOST;
}

double FSGModel::calcNodeCost(double input_vector[feat_num], const int &state)
{
	double result = 0;
	for (int c = 0; c < cluster_num; ++c) {
		double sum1 = 1, sum2 = 0;
		for (int n = 0; n < feat_num; ++n) {
			double sigma_square = covar[state][c][n];
			sum1 *= 2 * PI * sigma_square; // calc 2*pi*sigma
			sum2 += (input_vector[n] - means[state][c][n]) * (input_vector[n] - means[state][c][n]) / sigma_square;
		}
		result += weights[state][c] / sqrt(sum1) * pow((double)E, (-0.5) * sum2); 
	}
	return -log(result);
}

void FSGModel::drawGraph()
{
	FILE *file = fopen("graph.dot", "w");
	fprintf(file, "digraph G {\n");
	for (int i = 0; i <= end_state; ++i) {
		int son_num = son_num_of_state[i];
		for (int j = 0; j < son_num; ++j) {
			int son = son_of_state[i][j];
			fprintf(file, "%d->%d [label=\"%.10lf\"]\n", i, son, pow((double)E, -trans[i][j]));
			//fprintf(file, "%d [label=\"%d\"]\n", son, is_emit[son]);
		}
	}
	fprintf(file, "}\n");
	fclose(file);
}