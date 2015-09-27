#include <iostream>
#include <armadillo>
#include <math.h>
#include "readwave.h"

const double Pi = 3.1415;
typedef short SAMPLE;
typedef struct {
	SAMPLE* samples;
	int totalFrames;
	int sampleRate;
} audiodata; 
audiodata data;
char *FILENAME = "recordedData.wav";

// parameters
double wintime = 0.025;
double steptime = 0.010;
double preemph = 0.97;
int nbands = 40;
double minfreq = 133.33;
double maxfreq = 6855.5;

int round(double param)
{
	return (int)(param + 0.5);
}

// create a matrix with hamming value
arma::mat makeHamming(int size)
{
	arma::mat hamming(1, size);
	for (int i = 0; i < size; i++)
		hamming(0, i) = 0.54 - 0.46 * cos(2 * Pi * i / size);
	return hamming;
}

// calculate fft and then get the power
arma::mat powerFFT(arma::mat mat, int nfft)
{
	arma::cx_mat cmat = fft(mat, nfft);
	arma::mat result = square(real(cmat)) + square(imag(cmat));
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
	data.samples = ReadWave(filename, &(data.totalFrames), &(data.sampleRate));
}

arma::mat preemphasize()
{
	arma::mat result(1, data.totalFrames);
	result(0, 0) = (double)data.samples[0];
	for (int i = 1; i < data.totalFrames; i++)
		result(0, i) = data.samples[i] - preemph * data.samples[i - 1];
	return result;
}

// get a power spectrum from the samples
arma::mat getPowerSpec(arma::mat samples)
{
	int winpts = round(wintime * data.sampleRate); // points in a window
	int steppts = round(steptime * data.sampleRate); // points in a step
	int winnum = (data.totalFrames - winpts) / steppts + 1; // how many windows
	int nfft = pow(2.0, ceil(log((double)winpts) / log(2.0))); // fft numbers

	arma::mat powerSpec(nfft, winnum);
	arma::mat hamming = makeHamming(winpts);

	// for each window, do hamming window and get the power spectrum
	for (int i = 0; i < winnum; i++) {
		int start = i * steppts;
		int end = start + winpts - 1;
		arma::mat winsamples = samples.submat(0, start, 0, end);
		winsamples = winsamples % hamming; // element-wise multiplication, do hamming window
		powerSpec.col(i) = powerFFT(trans(winsamples), nfft); // do fft and get power spectrum
	}
	return powerSpec;
}

// get a matrix with weights of each frequency
arma::mat getFilterWeights(int nfft)
{
	arma::mat weights(nbands, nfft);

	double minmel = hz2mel(minfreq);
	double maxmel = hz2mel(maxfreq);
	arma::mat melfreq = minmel + arma::linspace<arma::mat>(0, nbands + 1, nbands + 2) / (nbands + 1) * (maxmel - minmel);
	arma::mat hzfreq = melfreq.transform(mel2hz);
	arma::mat point = round(hzfreq / data.sampleRate * nfft);

	// for each filter, calculate each weight
	for (int i = 0; i < nbands; i++) {
		weights.row(i).zeros();
		int start = point(i, 0), mid = point(i + 1, 0), end = point(i + 2, 0);
		int leftw = mid - start, rightw = end - mid;
		for (int j = start; j <= mid; j++)
			weights(i, j) = (j - start) / leftw;
		for (int j = mid + 1; j <= end; j++)
			weights(i, j) = (end - j) / rightw;
	}

	return weights;
}

int main()
{
	readSamplesFromFile(FILENAME);
	arma::mat preemphSamples = preemphasize();
	arma::mat powerSpec = getPowerSpec(preemphSamples);
	arma::mat audSpec = getFilterWeights(powerSpec.n_rows) * powerSpec;
	return 0;
}