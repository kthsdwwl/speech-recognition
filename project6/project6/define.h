#ifndef _DEF_H
#define _DEF_H

#define TYPE 2			   //whether it is single Gaussian(1) or GMM(2)

#define INF 0x5fffffff
#define PI 3.14159
#define E 2.71828l

#define beam 300
#define max_x 5000		   // max input length
#define max_y 500          // max total template length
#define max_bpt_node_num 5000
#define max_fsg_state_num 10
#define max_state_num 500
#define max_son_num 40
#define max_temp_num 150
#define max_model_num 20

#define train_temp_num 10  // the number of templates used for training
#define feat_num 39
#define max_state_size 5
#define state_num 5        // the number of word states, including the dummy state
#define silence_state_num 3
#define sil_id 11
#define cluster_num 2
#define EPSILON 0.2

#define EXITPROB 0.1
#define LOOPCOST 150

#define CORRSENPATH "sen_correct"
#define RECOSENPATH "sen_reco"

#define FSGMODELPATH1 "model/fsgmodel1"
#define FSGMODELPATH2 "model/fsgmodel2"

#define FSGMODELPATH "model/fsgmodel1"
#define WORDMODELPATH "model/wmodel2/"
#define TESTPATH "test1/"


#endif