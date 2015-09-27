#ifndef _FSG_MODEL_H
#define _FSG_MODEL_H

class FSGModel {
public:
	int fsg_state_num;
	int start_state;
	int end_state;
	int son_num_of_state[max_state_num];
	int son_of_state[max_state_num][max_son_num];
	double trans[max_state_num][max_son_num];
	double means[max_state_num][cluster_num][feat_num];
	double covar[max_state_num][cluster_num][feat_num];
	double weights[max_state_num][cluster_num];
	int is_emit[max_state_num];
	int fsg_state_index[max_state_num];

	void readFile(const char *filename);
	double calcNodeCost(double input_vector[feat_num], const int &state);
	void drawGraph();

};

#endif