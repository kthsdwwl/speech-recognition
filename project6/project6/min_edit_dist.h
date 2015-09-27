#ifndef _MIN_EIDT_DIST_H_
#define _MIN_EIDT_DIST_H_

#include <string.h>

namespace med {

/**
 * initialize functions
 */
void initInput(char *filename);

void initModel(const char *modelpath);

void delModel();

/**
 * calculate min edit distance
 */
double minEditDist();

/**
 * write segmentation results
 */
void writeResult(FILE *fp);

void outputTrellis();

}
#endif