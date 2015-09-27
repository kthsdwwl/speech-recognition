#ifndef _MIN_EIDT_DIST_W_H_
#define _MIN_EIDT_DIST_W_H_

#include <vector>
#include <string.h>
#include "model.h"

namespace medw {

typedef struct MatchData{
	int cost, best_index;

	MatchData(const int &_cost, const int &_index):
			 cost(_cost),
		     best_index(_index)
	{}

	void update(const int &_cost, const int &_index)
	{
		cost = _cost; 
		best_index = _index;
	}
} MatchData;

/**
 * initialize functions
 */
void addTemplate(const char* filename, const int &index);

void addModel(train::Model* model, const int &index);

void addModel(const char* filename, const int &index);

void initInput(const char* filename);

void initInput(double sequence[][feat_num], const int &len);

void outputEachTrellis(const int &k);

/**
 * update the segmentation of the model according to the DTW path
 */
void updateModelSegment(const int &index, const int &k, const int &k_len);

/**
 * calculate node and edge cost
 */
double nodeAndEdgeCost(const int &i, const int &k, const int &j, const int &pre_j);

/**
 * calculate min edit distance
 */
MatchData minEditDist();

}
#endif