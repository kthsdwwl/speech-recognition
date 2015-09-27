#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <fstream>
#include "model.h"
using namespace std;

long Model::writeToFile(const char* filename) {
	long offset = 0;
	FILE* file = fopen(filename, "wb");
	fwrite(trans, sizeof(trans), 1, file);
	offset += sizeof(trans);
	/*fwrite(seg_range, sizeof(seg_range), 1, file);
	offset += sizeof(seg_range);*/
	fwrite(seg_size, sizeof(seg_size), 1, file);
	offset += sizeof(seg_size);
	fclose(file);
	return offset;
}

long Model::readFromFile(const char* filename) {
	long offset = 0;
	FILE* file = fopen(filename, "rb");
	fread(trans, sizeof(trans), 1, file);
	offset += sizeof(trans);
	/*fread(seg_range, sizeof(seg_range), 1, file);
	offset += sizeof(seg_range);*/
	fread(seg_size, sizeof(seg_size), 1, file);
	offset += sizeof(seg_size);
	fclose(file);
	return offset;
}

void Model::updateTrans(double input[][max_x][feat_num], 
						int seg_range[][max_state_size][max_sen_words*2], 
						int word_in_sen[], 
						const int &sen_num) 
{
	seg_size[0] = 0;
	int to_next = 0, to_next_two = 0;
	for (int j = 0; j < sen_num; ++j) {
		int times = word_in_sen[j]; // how many times this word appears in sentence j
		seg_size[0] += times; // how many times this word appears in all the sentences
		for (int k = 0; k < times; ++k) {
			if (seg_range[j][1][2*k] != -1) // 0->1
				++to_next;
			else if (seg_range[j][2][2*k] != -1) // 0->2
				++to_next_two;
		}
	}
	double next_prob = (double)to_next / seg_size[0];
	double next_two_prob = (double)to_next_two / seg_size[0];
	trans[0][0] = INF;
	trans[0][1] = (next_prob == 0 ? INF : -log(next_prob));
	trans[0][2] = (next_two_prob == 0 ? INF : -log(next_two_prob));

	for (int i = 1; i < state_n; ++i) { 
		to_next = 0; to_next_two = 0;
		for (int j = 0; j < sen_num; ++j) {
			int times = word_in_sen[j]; // how many times this word appears in sentence j
			for (int k = 0; k < times; ++k) {
				if (seg_range[j][i][2*k] == -1) continue;
				if (i + 1 < state_n && seg_range[j][i+1][2*k] != -1)
					++to_next;
				else if (i + 2 < state_n && seg_range[j][i+2][2*k] != -1)
					++to_next_two;
			}
 		}
		double self_prob = (double)(seg_size[i] - to_next - to_next_two) / seg_size[i];
		double next_prob = (double)to_next / seg_size[i];
		double next_two_prob = (double)to_next_two / seg_size[i];
		trans[i][i] = (self_prob == 0 ? INF : -log(self_prob));
		if (i + 1 < state_n) trans[i][i + 1] = (next_prob == 0 ? INF : -log(next_prob));
		if (i + 2 < state_n) trans[i][i + 2] = (next_two_prob == 0 ? INF : -log(next_two_prob));
	}
}

long GMModel::writeToFile(const char* filename) {
	long offset = Model::writeToFile(filename);
	FILE* file = fopen(filename, "ab");
	fseek(file, offset, SEEK_SET);
	fwrite(means, sizeof(means), 1, file);
	offset += sizeof(means);
	fwrite(covar, sizeof(covar), 1, file);
	offset += sizeof(covar);
	fwrite(weights, sizeof(weights), 1, file);
	offset += sizeof(weights);
	fclose(file);
	return offset;
}

long GMModel::readFromFile(const char* filename) {
	long offset = Model::readFromFile(filename);
	FILE* file = fopen(filename, "rb");
	fseek(file, offset, SEEK_SET);
	fread(means, sizeof(means), 1, file);
	offset += sizeof(means);
	fread(covar, sizeof(covar), 1, file);
	offset += sizeof(covar);
	fread(weights, sizeof(weights), 1, file);
	offset += sizeof(weights);
	fclose(file);
	return offset;
}

void GMModel::updateMeansAndCov(double input[][max_x][feat_num], int seg_range[][max_state_size][max_sen_words*2], int word_in_sen[], const int &sen_num) {
	for (int i = 1; i < state_n; ++i) {
		//printf("Size of state %d: %d\n", i, seg_size[i]);
		int cur_num = 1; // current GMM cluster number  
		for (int j = 0; j < feat_num; ++j) {
			double sum = 0, sqare_sum = 0;
			for (int k = 0; k < sen_num; ++k) {
				int times = word_in_sen[k];
				//printf("state: %d, sentence: %d, times: %d\n", i, k, times);
				for (int t = 0; t < times; ++t) {
					if (seg_range[k][i][2*t] == -1) continue;
					for (int l = seg_range[k][i][2*t]; l <= seg_range[k][i][2*t+1]; ++l) {
						sum += input[k][l][j];
						sqare_sum += input[k][l][j] * input[k][l][j];
					}
				}
			}
			means[i][0][j] = sum / seg_size[i];
			covar[i][0][j] = sqare_sum / seg_size[i] - means[i][0][j] * means[i][0][j];
			//printf("sum: %lf, state: %d, dimension %d, means: %lf\n",sum, i, j, means[i][0][j]);
		}
		weights[i][0] = 1;
		while (cur_num < cluster_num) {
			// split
			for (int k = 0; k < cur_num; ++k) {
				for (int j = 0; j < feat_num; ++j) {
					means[i][k + cur_num][j] = means[i][k][j] + EPSILON;
					means[i][k][j] -= EPSILON;
					covar[i][k + cur_num][j] = covar[i][k][j];
				}
				weights[i][k] /= 2;
				weights[i][k + cur_num] = weights[i][k];
			}
			cur_num *= 2;
			// k-means
			kMeansOnState(i, cur_num, input, seg_range, word_in_sen, sen_num);
			// EM
			//EMOnState(i, cur_num, templates);
		}
	}
}

double GMModel::calcNodeCost(double input_vector[feat_num], const int &state) {
	if (state == 0) return INF;
	double result = 0;
	for (int c = 0; c < cluster_num; ++c) {
		double sum1 = 1, sum2 = 0;
		for (int n = 0; n < feat_num; ++n) {
			double sigma_square = covar[state][c][n];
			sum1 *= 2 * PI * sigma_square; // calc 2*pi*sigma
			sum2 += (input_vector[n] - means[state][c][n]) * (input_vector[n] - means[state][c][n]) / sigma_square;
		}
		result += weights[state][c] / sqrt(sum1) * pow((double)E, (-0.5) * sum2); 
	}
	return -log(result);
}

double GMModel::calcClusterDist(double input_vector[feat_num], const int &state, const int &c_index) {
	double sum1 = 0, sum2 = 0;
	for (int n = 0; n < feat_num; ++n) {
		double sigma_square = covar[state][c_index][n];
		sum1 += log(2 * PI * sigma_square); // calc log 2*pi*sigma
		sum2 += (input_vector[n] - means[state][c_index][n]) * (input_vector[n] - means[state][c_index][n]) / sigma_square;
	}
	return 0.5 * (sum1 + sum2) - log(weights[state][c_index]);
}

void GMModel::kMeansOnState(const int &state, 
							const int &cur_num, 
							double input[][max_x][feat_num], 
							int seg_range[][max_state_size][max_sen_words*2], 
							int word_in_sen[], 
							const int &sen_num) 
{
	int iter_time = 0;
	while (1) {
		double total_diff = 0;
		double temp_means[cluster_num][feat_num] = {0};
		double temp_covar[cluster_num][feat_num] = {0};
		int cluster_size[cluster_num] = {0};
		for (int k = 0; k < sen_num; ++k) {
			int times = word_in_sen[k];
			for (int t = 0; t < times; ++t) {
				if (seg_range[k][state][2*t] == -1) continue;
				for (int l = seg_range[k][state][2*t]; l <= seg_range[k][state][2*t+1]; ++l) {
					double min_dist = INF; int min_c;
					for (int c = 0; c < cur_num; ++c) {
						// calculate the distance between the vector and the cluster
						double dist = calcClusterDist(input[k][l], state, c); 
						if (dist < min_dist) {
							min_dist = dist;
							min_c = c;
						}
					}
					cluster_size[min_c]++;
					for (int j = 0; j < feat_num; ++j) {
						temp_means[min_c][j] += input[k][l][j];
						temp_covar[min_c][j] += input[k][l][j] * input[k][l][j];
					}
				}
			}
		}
		for (int c = 0; c < cur_num; ++c) {
			double temp_weights = weights[state][c];
			weights[state][c] = (double)cluster_size[c] / seg_size[state];
			total_diff += fabs(weights[state][c] - temp_weights);
			for (int j = 0; j < feat_num; ++j) {
				means[state][c][j] = temp_means[c][j] / cluster_size[c];
				covar[state][c][j] = temp_covar[c][j] / cluster_size[c] - means[state][c][j] * means[state][c][j];
			}
		}
		if (iter_time > 5 && total_diff == 0)
			break;
		++iter_time;
		//printf("Kmeans iter no. %d, %lf\n", iter_time, total_diff);
	}
}

void GMModel::EMOnState(const int &state, 
						const int &cur_num, 
						double input[][max_x][feat_num], 
						int seg_range[][max_state_size][max_sen_words*2], 
						int word_in_sen[], 
						const int &sen_num) 
{
	int iter_time = 0;
	while (1) {
		double temp_means[cluster_num][feat_num] = {0};
		double temp_covar[cluster_num][feat_num] = {0};
		double cluster_size[cluster_num] = {0};
		double cluster_prob[400][cluster_num];
		int point_index = 0; // the index of the point in this state
		for (int k = 0; k < sen_num; ++k) {
			int times = word_in_sen[k];
			for (int t = 0; t < times; ++t) {
				if (seg_range[k][state][2*t] == -1) continue;
				for (int l = seg_range[k][state][2*t]; l <= seg_range[k][state][2*t+1]; ++l) {
					double sum_prob = 0;
					// for each point, calculate the probability it is produced by each cluster
					for (int c = 0; c < cur_num; ++c) {
						double prob = pow((double)E, -calcClusterDist(input[k][l], state, c));
						cluster_prob[point_index][c] = weights[state][c] * prob;
						sum_prob += cluster_prob[point_index][c];
					}
					for (int c = 0; c < cur_num; ++c) {
						cluster_prob[point_index][c] /= sum_prob;
						cluster_size[c] += cluster_prob[point_index][c];
						// add the point value to the means array 
						for (int j = 0; j < feat_num; ++j)
							temp_means[c][j] += cluster_prob[point_index][c] * input[k][l][j];
					}
					point_index++; // go to next point
				}
			}
		}
		// update weights and means for each cluster
		for (int c = 0; c < cur_num; ++c) {
			weights[state][c] = cluster_size[c] / seg_size[state];
			for (int j = 0; j < feat_num; ++j) 
				means[state][c][j] = temp_means[c][j] / cluster_size[c];
		}
		// update covariance
		point_index = 0;
		for (int k = 0; k < sen_num; ++k) {
			int times = word_in_sen[k];
			for (int t = 0; t < times; ++t) {
				if (seg_range[k][state][2*t] == -1) continue;
				for (int l = seg_range[k][state][2*t]; l <= seg_range[k][state][2*t+1]; ++l) {
					for (int c = 0; c < cur_num; ++c)
						for (int j = 0; j < feat_num; ++j)
							temp_covar[c][j] += cluster_prob[point_index][c] * (input[k][l][j] - means[state][c][j]) * (input[k][l][j] - means[state][c][j]);
					point_index++;
				}
			}
		}
		for (int c = 0; c < cur_num; ++c) {
			for (int j = 0; j < feat_num; ++j)
				covar[state][c][j] = temp_covar[c][j] / cluster_size[c];
		}
		if (iter_time > 100)
			break;
		++iter_time;
	}
}