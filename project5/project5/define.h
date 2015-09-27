#ifndef _DEFINE_H
#define _DEFINE_H

//parameters
#define TYPE 0 // 0 is beam search
#define beam 8
#define threshold 3
#define inf 0x7fffffff
#define max_x 650
#define max_y 650
#define max_word_len 30
#define max_temp_num 7000
#define max_son_num 52
#define max_word_num 7000
#define max_line_num 5
#define max_line_len 650
#define max_node_num 30000
#define max_story_word_num 650
#define max_text_len 650

//0: do not use language model
//1: use unigram
//2: use bigram
#define lang_model_choice 1

//paths
#define MODELTRAINPATH "storycorrect.txt"

#define SFILENAME "typos.txt"
#define MSFILENAME "mystory.txt"
#define CSFILENAME "storycorrect.txt"
#define UNSEGFILENAME "unsegmented.txt"
#define SEGRESULTFILENAME "mysegmented.txt"
#define CSEGFILENAME "segmented.txt"
#define DICTPATH "dict_1.txt"
#define LEXPATH "lextrees.bin"
#define DICTPATHTEST "dict_1_test.txt"
#define LEXPATHTEST "lextrees_test.bin"
#define GRAPHPATH "graph.dot"

#endif