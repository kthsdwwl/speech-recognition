#include "define.h"
#include "fsgmodel.h"
#include "genfsg.h"
#include "strmatch.h"
#include "min_edit_dist.h"
#include <math.h>

int correctSentence(const char *orifile, const char *recofile)
{
	FILE *fp1 = fopen(orifile, "r");
	FILE *fp2 = fopen(recofile, "r");

	int ret = 0;
	char str1[50], str2[50];
	while (fscanf(fp1, "%s", str1) == 1) {
		fscanf(fp2, "%s", str2);
		//printf("%s %s\n", str1, str2);
		if (strcmp(str1, str2) == 0)
			++ret;
	}

	fclose(fp1);
	fclose(fp2);
	return ret;
}

int correctDigit(const char *orifile, const char *recofile)
{
	FILE *fp1 = fopen(orifile, "r");
	FILE *fp2 = fopen(recofile, "r");

	int ret = 0;
	char str1[50], str2[50];
	while (fscanf(fp1, "%s", str1) == 1) {
		fscanf(fp2, "%s", str2);
		ret += correctChar(str1, str2);
	}

	fclose(fp1);
	fclose(fp2);
	return ret;
}

int totalSentences(const char *filename)
{
	int ret = 0;
	char str[50];
	FILE *fp = fopen(filename, "r");
	while (fscanf(fp, "%s", str) == 1) ++ret;
	return ret;
}

int totalDigits(const char *filename)
{
	int ret = 0;
	char str[50];
	FILE *fp = fopen(filename, "r");
	while (fscanf(fp, "%s", str) == 1) ret += strlen(str);
	return ret;
}

void sentenceCorrectness()
{
	printf("==========\n");
	int sn = totalSentences(CORRSENPATH), csn = correctSentence(CORRSENPATH, RECOSENPATH);
	printf("The total number of sentences is %d\n", sn);
	printf("The number of correct sentences is %d\n", csn);
	printf("The accuracy is %.2lf%%\n", (double)csn/sn*100);
	printf("==========\n");
}

void digitCorrectness()
{
	printf("==========\n");
	int dn1 = totalDigits(CORRSENPATH), dn2 = totalDigits(RECOSENPATH), cdn = correctDigit(CORRSENPATH, RECOSENPATH);
	printf("The total number of test digit is %d\n", dn1);
	printf("The total number of recognized digit is %d\n", dn2);
	printf("The number of correct digit is %d\n", cdn);
	printf("The accuracy is %.2lf%%\n", (double)cdn/dn1*100);
	printf("Length difference is %.2lf%%\n", (double)(abs(dn1-dn2))/dn1*100);
	printf("==========\n");
}

void dealWithFilename(const char filename[], char result[])
{
	int namelen = strlen(filename), result_len = 0;
	for (int i = 0; i < namelen; ++i)
	{
		if (filename[i] == 'O' || filename[i] == 'Z')
			result[result_len++] = '0';
		else if (filename[i] >= '0' && filename[i] <= '9')
			result[result_len++] = filename[i];
	}
	result[result_len] = '\0';
}

void testWav(const char *testpath)
{
	char filelist[40], wavpath[40];
	strcpy(filelist, testpath);
	strcpy(wavpath, testpath);
	strcat(filelist, "TEST.filelist");
	strcat(wavpath, "testwav/");

	FILE *fp1 = fopen(CORRSENPATH, "w");
	FILE *fp2 = fopen(RECOSENPATH, "w");
	med::initModel(FSGMODELPATH);

	FILE* pFile;
	pFile = fopen(filelist , "r");
	if (pFile == NULL) printf("Error opening file\n");

	char filename[30], wav_path[40], transfilename[30];
	while (fgets(filename, 20, pFile))
	{
		filename[strlen(filename) - 1] = '\0';
		dealWithFilename(filename, transfilename);
		fprintf(fp1, "%s\n", transfilename);

		strcpy(wav_path, wavpath);
		strcat(strcat(wav_path, filename), ".wav");
		printf("Matching file: %s\n", filename);
		med::initInput(wav_path);
		med::minEditDist();
		med::writeResult(fp2);
		//med::outputTrellis();
		strcpy(filename, "");
	}
	fclose(pFile);

	med::delModel();
	fclose(fp1);
	fclose(fp2);
}

int main()
{
	//generateFSG1();
	//generateFSG2();
	testWav(TESTPATH);

	sentenceCorrectness();
	digitCorrectness();
	return 0;
}