#ifndef _DEF_H_
#define _DEF_H_

/**
 * parameters
 */
#define TYPE 2			   //whether it is a simple DTW(0) or single Gaussian(1) or GMM(2)

#define INF 0x7fffffff
#define PI 3.14159
#define E 2.71828l
#define EPSILON 0.2

#define beam 1000
#define max_x 800		   // max input length
#define max_y 50000        // max total template length
#define max_temp_num 150
#define max_model_num 20

#define feat_num 39
#define max_state_size 5
#define state_num 5        // the number of states, including the dummy state
#define silence_state_num 3

#define max_sen_num 7000   // the number of templates used for training
#define max_sen_state_num 50
#define max_train_words 12
#define max_sen_words 8
#define cluster_num 1

#define eps 0.00001        // stop iteration

#define ORIMODELPATH "orimodels/"
#define RESMODELPATH "models1/"
#define TRAINFILELIST "train1/TRAIN.filelist"
#define TRAINWAVPATH "train1/trainwav/"
#define TRAINFEATPATH "train1/trainfeat/"

#endif