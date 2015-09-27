#ifndef _MIN_EIDT_DIST_W_H_
#define _MIN_EIDT_DIST_W_H_

#include <vector>
#include <string.h>
#include "model.h"

namespace medw {

/**
 * initialize functions
 */
void initparam();

int initTemplatesForSen(const char *filelist, const char *train_path);

//void addTemplate(const char* filename, const int &index);

void outputEachTrellis(const int &index);

/**
 * update functions
 */
void updateSegmentation(GMModel* models[], const int &index);

void updateMeanCovTrans(int sen_num, GMModel* models[]);

/**
 * calculate node and edge cost
 */
double nodeAndEdgeCost(GMModel* models[], const int &index, const int &i, const int &k, const int &j, const int &pre_j);

/**
 * calculate min edit distance
 */
double minEditDist(int index, GMModel* models[]);

/**
 * helper
 */
void dealWithFilename(int &index, char *file_name);
}
#endif