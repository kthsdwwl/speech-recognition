#ifndef _MODEL_H_
#define _MODEL_H_

#include "define.h"


class Model{
public:
	int state_n;
	double trans[max_state_size][max_state_size];
	int seg_size[max_state_size];                     // number of points in one segment
	//int seg_range[max_sen_num][state_num][2];    // range of segment in each template

	//virtual void init(int temp_len[], double templates[][max_x][feat_num]);

	virtual long writeToFile(const char* filename);

	virtual long readFromFile(const char* filename);

	//virtual void writeSegmentToFile(const char* filename, double templates[][max_x][feat_num]);

	//virtual void updateMeansAndCov(double input[][max_x][feat_num], int seg_range[][max_state_size][max_sen_words*2], int word_in_sen[], const int &sen_num) = 0;

	//virtual double calcNodeCost(double input_vector[feat_num], const int &state) = 0;
	
	/**
	 * update the state translation probability
	 */
	//void updateTrans(double input[][max_x][feat_num], int seg_range[][max_state_size][max_sen_words*2], int word_in_sen[], const int &sen_num);

	/**
	 * update segmentation according to the match result
	 */
	//void updateSegment(const int &k, const int &k_len, int arrow[][max_sen_state_num]);

} ;

//class SGModel : public Model {
//public:
//	double means[state_num][feat_num];
//	double covar[state_num][feat_num];
//	
//	void init(int temp_len[], double templates[][max_x][feat_num]);
//
//	long writeToFile(const char* filename);
//
//	long readFromFile(const char* filename);
//
//	/**
//	 * update means and variances for each state
//	 */
//	void updateMeansAndCov(double templates[][max_x][feat_num]);
//
//	double calcNodeCost(double input_vector[feat_num], const int &state);
//};

class GMModel : public Model {
public:
	double means[max_state_size][cluster_num][feat_num];
	double covar[max_state_size][cluster_num][feat_num];
	double weights[max_state_size][cluster_num];
	
	//void init(int temp_len[], double templates[][max_x][feat_num]);

	long writeToFile(const char* filename);

	long readFromFile(const char* filename);

	//virtual void writeSegmentToFile(const char* filename, double templates[][max_x][feat_num]);

	/**
	 * update means and variances for each state
	 */
	//void updateMeansAndCov(double input[][max_x][feat_num], int seg_range[][max_state_size][max_sen_words*2], int word_in_sen[], const int &sen_num);

	//double calcNodeCost(double input_vector[feat_num], const int &state);

private:
	//double calcClusterDist(double input_vector[feat_num], const int &state, const int &c_index);

	//void kMeansOnState(const int &state, const int &cur_num, double input[][max_x][feat_num], int seg_range[][max_state_size][max_sen_words*2], int word_in_sen[], const int &sen_num);

	//void EMOnState(const int &state, const int &cur_num, double input[][max_x][feat_num], int seg_range[][max_state_size][max_sen_words*2], int word_in_sen[], const int &sen_num);
};


#endif