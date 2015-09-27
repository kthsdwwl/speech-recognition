#include <stdio.h>
#include <string.h>
#include <math.h>
#include "model.h"
#include "train.h"
#include "min_edit_dist.h"

namespace train {

GMModel* models[max_model_num];

/**
 * do trainint
 */
void doTraining(const char *filelist, const char *train_path) 
{
	double last_cost=-INF;
	double current_cost;
	//int temp_seg_size[max_train_words][state_num];
	int sen_num = medw::initTemplatesForSen(filelist, train_path);
	for (int i = 0; i < max_train_words; ++i)
	{
		char num[3];
		char model_path[20];
		itoa(i, num, 10);
		strcpy(model_path, ORIMODELPATH);
		strcat(strcat(model_path, "model_"), num);
		//printf("Now reading model %s\n", model_path);
		models[i] = new GMModel();
		models[i]->readFromFile(model_path);

		if (i != 11) models[i]->state_n = state_num;
		else models[i]->state_n = silence_state_num;
	}
	int iter = 0;
	while (1) {
		current_cost = 0;

		// update segment size
		medw::initparam();
		for (int i = 0; i < max_train_words; ++i)
			memset(models[i]->seg_size, 0, sizeof(models[i]->seg_size));

		for (int i = 0; i < sen_num; ++i) {
			//printf("Now matching sentence %d\n", i);
			double temp = medw::minEditDist(i, models);
			if (i == 28)
			medw::outputEachTrellis(i);
			current_cost += temp;
			//printf("cost: %lf\n", temp);
			medw::updateSegmentation(models, i);
		}
		
		printf("Iteration no %d, cost is %lf\n", iter, current_cost);

		// whether to stop
		printf("%lf\n", (current_cost - last_cost)/current_cost);
		if (fabs((current_cost - last_cost)/current_cost) < eps)
			break;
		if (fabs(current_cost - last_cost) < eps)
			break;

		last_cost = current_cost;
		medw::updateMeanCovTrans(sen_num, models);

		++iter;
	}

	// write model to file
	for (int i = 0; i < max_train_words; ++i)
	{
		char num[3];
		char model_path[40];
		itoa(i, num, 10);
		strcpy(model_path, RESMODELPATH);
		strcat(strcat(model_path, "model_"), num);
		//printf("%s\n", model_path);
		models[i]->writeToFile(model_path);
	}
	//model->writeToFile(strcat(filename, word));
}

}