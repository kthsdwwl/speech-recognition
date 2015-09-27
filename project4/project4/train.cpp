#include <stdio.h>
#include <string.h>
#include <math.h>
#include "model.h"
#include "train.h"
#include "min_edit_dist.h"

namespace train {

double templates[train_temp_num][max_x][feat_num];
int temp_len[train_temp_num];

/**
 * read all templates for the specific word
 */
void initTemplatesForWord(char word[])
{
	char postfix[15][5] = {"0", "1", "2", "3", "4", "5",
						 "6", "7", "8", "9", "10", 
						 "11", "12", "13", "14"};
	for (int k = 0; k < train_temp_num; ++k) {
		char filename[40] = "templates/";
		strcat(filename, word);
		strcat(strcat(strcat(filename, "_"), postfix[k]), ".txt");
		FILE* file = fopen(filename, "r");
		
		int win_size;
		fscanf(file, "%d", &win_size);
		temp_len[k] = win_size;
		for (int j = 0; j < feat_num; ++j) {
			templates[k][0][j] = 0;
			for (int i = 1; i <= win_size; ++i) {
				fscanf(file, "%lf", &templates[k][i][j]);
			}
		}

		fclose(file);
	}
}

/**
 * do trainint
 */
void doTraining(char word[]) 
{
	int align_error;
	int temp_seg_size[state_num];
	initTemplatesForWord(word);
	Model* model;

	if (TYPE == 1)
		model = new SGModel();
	else if (TYPE == 2)
		model = new GMModel();
	else return;

	model->init(temp_len, templates);
	medw::addModel(model, 0);
	int iter = 0;
	while (1) {
		align_error = 0;

		// save segment size
		for (int i = 0; i < state_num; ++i) temp_seg_size[i] = model->seg_size[i];

		// update segment size
		memset(model->seg_size, 0, sizeof(model->seg_size));
		for (int i = 0; i < train_temp_num; ++i) {
			medw::initInput(templates[i], temp_len[i]);
			medw::minEditDist();
			medw::updateModelSegment(0, i, temp_len[i]);
		}

		// calc alignment error
		for (int i = 0; i < state_num; ++i) {
			//printf("state %d: %d  ", i, model->seg_size[i]);
			align_error += abs(temp_seg_size[i] - model->seg_size[i]);
		}
		//printf("\nword: %s, diff: %d\n", word, align_error);
		if (align_error <= 2)
			break;

		model->updateMeansAndCov(templates);
		model->updateTrans(templates);
	}
	// write model to file
	char filename[40] = "models/model_";
	model->writeToFile(strcat(filename, word));
}

}