#include <armadillo>
#include "endpointing.h"
#include "wav2feat.h"
using namespace arma;

const double PI = 3.1415;
typedef short SAMPLE;
typedef struct {
	SAMPLE* samples;
	int totalFrames;
	int sampleRate;
} audioinfo; 
audioinfo ainfo;
std::fstream file; 

char *FILENAME = "recordedData.wav";
// parameters
const double wintime = 0.025;
const double steptime = 0.010;
const double preemph = 0.97;
const int nbands = 40;
const double minfreq = 133.33;
const double maxfreq = 6855.5;
const int ndct = 13;

int round(double param)
{
	return (int)(param + 0.5);
}

// create a matrix with hamming value
mat makeHamming(int size)
{
	mat hamming(1, size);
	for (int i = 0; i < size; i++)
		hamming(0, i) = 0.54 - 0.46 * cos(2 * PI * i / size);
	return hamming;
}

// calculate fft and then get the power
mat powerFFT(mat inputmat, int nfft)
{
	cx_mat cmat = fft(inputmat, nfft);
	mat result = square(real(cmat)) + square(imag(cmat));
	return result;
}

double hz2mel(double hz)
{
	return 2595 * log10(1 + hz / 700);
}

double mel2hz(double mel)
{
	return 700 * (pow(10, mel / 2595) - 1);
}

void readSamplesFromFile(char *filename)
{
	ainfo.samples = ReadWave(filename, &(ainfo.totalFrames), &(ainfo.sampleRate));
}

mat preemphasize()
{
	mat result(1, ainfo.totalFrames);
	result(0, 0) = (double)ainfo.samples[0];
	for (int i = 1; i < ainfo.totalFrames; i++)
		result(0, i) = ainfo.samples[i] - preemph * ainfo.samples[i - 1];
	return result;
}

// get a power spectrum from the samples
mat getPowerSpec(mat samples)
{
	int winpts = round(wintime * ainfo.sampleRate); // points in a window
	int steppts = round(steptime * ainfo.sampleRate); // points in a step
	int winnum = (ainfo.totalFrames - winpts) / steppts + 1; // how many windows
	int nfft = pow(2.0, ceil(log((double)winpts) / log(2.0))); // fft numbers

	mat powerSpec(nfft, winnum);
	mat hamming = makeHamming(winpts);

	// for each window, do hamming window and get the power spectrum
	for (int i = 0; i < winnum; i++) {
		int start = i * steppts;
		int end = start + winpts - 1;
		mat winsamples = samples.submat(0, start, 0, end);
		winsamples = winsamples % hamming; // element-wise multiplication, do hamming window
		powerSpec.col(i) = powerFFT(trans(winsamples), nfft); // do fft and get power spectrum
	}
	return powerSpec;
}

// get a matrix with weights of each frequency
mat getFilterWeights(int nfft)
{
	mat weights(nbands, nfft);

	double minmel = hz2mel(minfreq);
	double maxmel = hz2mel(maxfreq);
	mat melfreqs = minmel + linspace<mat>(0, nbands + 1, nbands + 2) / (nbands + 1) * (maxmel - minmel);
	mat hzfreqs = melfreqs.transform(mel2hz);
	mat fftfreqs = linspace<mat>(0, nfft - 1, nfft) / nfft * ainfo.sampleRate;

	// for each filter, calculate each weight
	for (int i = 0; i < nbands; i++) {
		weights.row(i).zeros();
		int start = hzfreqs(i, 0), mid = hzfreqs(i + 1, 0), end = hzfreqs(i + 2, 0);

		// lower and upper slopes for all bins
		mat loslope = trans((fftfreqs - start) / (mid - start));
		mat hislope = trans((end - fftfreqs) / (end - mid));
		weights.row(i) = max(zeros<mat>(1, nfft), min(loslope, hislope));
	}

	return weights;
}

mat dct(mat audSpec)
{
	mat DCTCoeff(ndct, nbands);
	for (int i = 0; i < ndct; i++) 
		for (int j = 0; j < nbands; j++)
			DCTCoeff(i, j) = sqrt(2.0 / nbands) * cos(PI / nbands * i * (j + 0.5));
	DCTCoeff.row(0) /= sqrt(2.0);
	return DCTCoeff * audSpec;
}

mat normalize(mat inputmat)
{
	mat result = inputmat;
	mat meanmat = mean(inputmat, 1);
	result.each_col() -= meanmat;

	mat stddevmat = stddev(inputmat, 0, 1);
	result.each_col() /= stddevmat;
	return result;
}

mat calcTempVar(mat inputmat)
{
	int x = inputmat.n_rows, y = inputmat.n_cols;
	mat result(x * 3, y);
	result.rows(0, x - 1) = inputmat;
	// velocity
	for (int j = 1; j < y - 1; j++)
		result.rows(x, x * 2 - 1).col(j) = result.rows(0, x - 1).col(j + 1) - result.rows(0, x - 1).col(j - 1);
	result.rows(x, x * 2 - 1).col(0) = result.rows(x, x * 2 - 1).col(1);
	result.rows(x, x * 2 - 1).col(y - 1) = result.rows(x, x * 2 - 1).col(y - 2);

	// acceleration
	for (int j = 1; j < y - 1; j++)
		result.rows(x * 2, x * 3 - 1).col(j) = result.rows(x, x * 2 - 1).col(j + 1) - result.rows(x, x * 2 - 1).col(j - 1);
	result.rows(x * 2, x * 3 - 1).col(0) = result.rows(x * 2, x * 3 - 1).col(1);
	result.rows(x * 2, x * 3 - 1).col(y-1) = result.rows(x * 2, x * 3 - 1).col(y-2);
	return result;
}

void outputToFile(mat source, char *filename)
{
	remove(filename);
	file.open(filename, std::ios::app);
	file << source.n_cols << "\n";
	for (int i = 0; i < source.n_rows; i++) {
		for (int j = 0; j < source.n_cols; j++)
			file << source(i, j) << " ";
		file << "\n";
	}
	
	file.close();
}

void captureFeatAndWriteToFile(char* featpath)
{
	if (ainfo.totalFrames <= 0)
		return;

	//printf("=== Capturing features... ===\n");
	mat preemphSamples = preemphasize();
	mat powerSpec = getPowerSpec(preemphSamples);
	mat audSpec = log(getFilterWeights(powerSpec.n_rows) * powerSpec); // calculate 40 filter values
	mat ceps = dct(audSpec); // get cepstrum
	mat featureVecs = normalize(calcTempVar(ceps));
	//printf("Done!\n\n");
	
	//printf("=== Writing files... ===\n");
	outputToFile(featureVecs, featpath);
	//printf("Done\n\n");
}

void wav2feat(char* featpath)
{
	waitForPressKey();

	// record audio
	initAudioData();
	initPa();
	captureAndEndpointing();
	writeAudioToFile();
	stopPa();

	// capture features
	readSamplesFromFile(FILENAME);
	captureFeatAndWriteToFile(featpath);
}

void wav2featbat(const char *filelist, const char *wavpath, const char *featpath)
{
	FILE* pFile;
	pFile = fopen(filelist , "r");
	if (pFile == NULL) printf("Error opening file\n");

	char filename[30], wav_path[30], feat_path[30];
	while (fgets(filename, 20, pFile))
	{
		filename[strlen(filename) - 1] = '\0';
		strcpy(wav_path, wavpath);
		strcpy(feat_path, featpath);
		strcat(strcat(wav_path, filename), ".wav");
		strcat(feat_path, filename);
		readSamplesFromFile(wav_path);
		captureFeatAndWriteToFile(feat_path);
		strcpy(filename, "");
	}
	fclose(pFile);
}