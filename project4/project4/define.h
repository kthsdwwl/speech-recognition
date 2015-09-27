#ifndef _DEF_H_
#define _DEF_H_

/**
 * parameters
 */
#define TYPE 2			   //whether it is a simple DTW(0) or single Gaussian(1) or GMM(2)

#define inf 0x7fffffff
#define Pi 3.14159
#define E 2.71828l

#define beam 1000
#define max_x 400		   // max input length
#define max_y 50000        // max total template length
#define max_temp_num 150
#define max_model_num 20

#define feat_num 39
#define max_state_size 5
#define state_num 3        // the number of states, including the dummy state
#define train_temp_num 10  // the number of templates used for training

#define cluster_num 4
#define EPSILON 0.2

#endif