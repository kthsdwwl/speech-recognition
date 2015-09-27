#include <stdio.h>
#include "define.h"

void generateFSG1()
{
	int start_state = 0;
	int end_state;
	int fsg_state_num = 8;
	int fsg_state[max_fsg_state_num];
	fsg_state[0] = start_state;
	fsg_state[1] = fsg_state[0] + (state_num-1) * 8 + (silence_state_num-1) + 1;
	for (int i = 2; i < fsg_state_num; ++i)
		fsg_state[i] = fsg_state[i-1] + (state_num-1) * 10 + 1;
	end_state = fsg_state[fsg_state_num-1];

	FILE *file = fopen(FSGMODELPATH1, "w");

	fprintf(file, "%d\n%d\n%d\n", fsg_state_num, start_state, end_state);
	for (int i = 0; i < fsg_state_num; ++i)
		fprintf(file, "%d ", fsg_state[i]);
	fprintf(file, "\n");

	// write each edge
	fprintf(file, "%d %d %d\n", fsg_state[0], fsg_state[0], 11); // silence edge
	for (int j = 2; j < 10; ++j)
		fprintf(file, "%d %d %d\n", fsg_state[0], fsg_state[1], j);
	for (int i = 1; i < fsg_state_num-1; ++i)
		for (int j = 0; j < 10; ++j)
			fprintf(file, "%d %d %d\n", fsg_state[i], fsg_state[i+1], j);

	// empty edge
	fprintf(file, "%d %d %d\n", fsg_state[0], fsg_state[3], -1);
	fclose(file);
}

void generateFSG2()
{
	int start_state = 0;
	int end_state;
	int fsg_state_num = 2;
	int fsg_state[max_fsg_state_num];
	fsg_state[0] = start_state;
	fsg_state[1] = fsg_state[0] + (state_num-1) * 11 + (silence_state_num-1) + 1 ;
	end_state = fsg_state[fsg_state_num-1];

	FILE *file = fopen(FSGMODELPATH2, "w");

	fprintf(file, "%d\n%d\n%d\n", fsg_state_num, start_state, end_state);
	for (int i = 0; i < fsg_state_num; ++i)
		fprintf(file, "%d ", fsg_state[i]);
	fprintf(file, "\n");

	// write each edge
	fprintf(file, "%d %d %d\n", fsg_state[0], fsg_state[0], 11); // silence edge
	for (int j = 0; j < 11; ++j)
		fprintf(file, "%d %d %d\n", fsg_state[0], fsg_state[1], j);

	// empty edge
	fprintf(file, "%d %d %d\n", fsg_state[1], fsg_state[0], -1);
	fclose(file);
}