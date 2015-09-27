#include <time.h>
#include "train.h"
#include "min_edit_dist.h"
#include "wav2feat.h"

char nums[15][5] = {"0", "1", "2", "3", "4", "5",
						 "6", "7", "8", "9", "10", 
						 "11", "12", "13", "14"};
const int test_num = 12;

void readTemplates()
{
	int index = 0;
	for (int i = 0; i < test_num; ++i) { 
		for (int j = 0; j < train_temp_num; ++j) { 
			char filename[40] = "templates/";
			strcat(strcat(strcat(strcat(filename, nums[i]), "_"), nums[j]), ".txt");
			medw::addTemplate(filename, index++); 
		}
	}
}

void trainModels()
{
	printf("=== Training... ===\n");
	for (int i = 0; i < test_num; ++i)
		train::doTraining(nums[i]);
	printf("Done!\n\n");
}

void readModels()
{
	for (int i = 0; i < test_num; ++i) {
		char filename[40] = "models/model_";
		medw::addModel(strcat(filename, nums[i]), i);
	}
}

void testMatch()
{
	int total_error = 0;
	for (int i = 0; i < test_num; ++i) {                         // test words from zero to nine 
		printf("=== Now testing word %d... ===\n", i);
		for (int j = 0; j < 10; ++j) {							 // each word has 10 input
			char filename[40] = "input/";
			strcat(strcat(strcat(strcat(filename, nums[i]), "_"), nums[j]), ".txt");
			medw::initInput(filename);
			medw::MatchData data = medw::minEditDist();
			//medw::outputEachTrellis(0);
			if (TYPE == 0) {
				if (data.best_index / train_temp_num ==  i)
					printf("Input %d, match result: %d, correct!\n", j, i);
				else {
					++total_error;
					printf("Input %d, match result: %d, incorrect!\n", j, data.best_index / train_temp_num);
				}
			}
			else {
				if (TYPE != 0 && data.best_index == i)
					printf("Input %d, match result: %d, correct!\n", j, i);
				else {
					++total_error;
					printf("Input %d, match result: %d, incorrect!\n", j, data.best_index);
				}
			}
		}
	}
	printf("correct rate: %lf\n", (double)(test_num * 10 - total_error) / (test_num * 10));
}

void recordAndMatch()
{
	wav2feat();
	printf("=== Now matching... ===\n");
	medw::initInput("featureVecs.txt");
	medw::MatchData data = medw::minEditDist();
	printf("cost: %lf\n", data.cost);
	if (TYPE == 0) {
		printf("Match result: %d\n\n", data.best_index / train_temp_num);
	}
	else {
		printf("Match result: %d\n\n", data.best_index);
	}
}

int main()
{
	if (TYPE == 0) readTemplates();
	else {
		trainModels();
		readModels();
	}
	testMatch();
	recordAndMatch();

	return 0;
}