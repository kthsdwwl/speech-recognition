#include <time.h>
#include "train.h"
#include "min_edit_dist.h"
#include "wav2feat.h"

int main()
{

	//wav2featbat(TRAINFILELIST, TRAINWAVPATH, TRAINFEATPATH);
	train::doTraining(TRAINFILELIST, TRAINFEATPATH);

	return 0;
}