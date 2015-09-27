#ifndef _MODEL_H_
#define _MODEL_H_

#include "define.h"

namespace train {

class Model{
public:
	double trans[max_state_size][max_state_size];
	int seg_range[train_temp_num][max_state_size][2]; // range of segment in each template
	int seg_size[max_state_size];                     // number of points in one segment

	virtual void init(int temp_len[], double templates[][max_x][feat_num]);

	virtual long writeToFile(const char* filename);

	virtual long readFromFile(const char* filename);

	//virtual void writeSegmentToFile(const char* filename, double templates[][max_x][feat_num]);

	virtual void updateMeansAndCov(double templates[][max_x][feat_num]) = 0;

	virtual double calcNodeCost(double input_vector[feat_num], const int &state) = 0;
	
	/**
	 * update the state translation probability
	 */
	void updateTrans(double templates[][max_x][feat_num]);

	/**
	 * update segmentation according to the match result
	 */
	void updateSegment(const int &k, const int &k_len, int arrow[][max_y]);

} ;

class SGModel : public Model {
public:
	double means[max_state_size][feat_num];
	double covar[max_state_size][feat_num];
	
	void init(int temp_len[], double templates[][max_x][feat_num]);

	long writeToFile(const char* filename);

	long readFromFile(const char* filename);

	/**
	 * update means and variances for each state
	 */
	void updateMeansAndCov(double templates[][max_x][feat_num]);

	double calcNodeCost(double input_vector[feat_num], const int &state);
};

class GMModel : public Model {
public:
	double means[max_state_size][cluster_num][feat_num];
	double covar[max_state_size][cluster_num][feat_num];
	double weights[max_state_size][cluster_num];
	
	void init(int temp_len[], double templates[][max_x][feat_num]);

	long writeToFile(const char* filename);

	long readFromFile(const char* filename);

	//virtual void writeSegmentToFile(const char* filename, double templates[][max_x][feat_num]);

	/**
	 * update means and variances for each state
	 */
	void updateMeansAndCov(double templates[][max_x][feat_num]);

	double calcNodeCost(double input_vector[feat_num], const int &state);

private:
	double calcClusterDist(double input_vector[feat_num], const int &state, const int &c_index);

	void kMeansOnState(const int &state, const int &cur_num, double templates[][max_x][feat_num]);

	void EMOnState(const int &state, const int &cur_num, double templates[][max_x][feat_num]);
};

}

#endif