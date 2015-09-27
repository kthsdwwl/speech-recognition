#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include <fstream> 
#include "portaudio.h"
#include "readwave.h"

const double sampleRate = 16000;
const unsigned long framesPerBuffer = 512;

#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;

/* data structure recording audio data */
typedef struct {
	int frameIndex;
	int outputFrameIndex;
	int recordedFrameNum;
	int outputFrameNum;
	SAMPLE *recordedData;
	SAMPLE *outputData;
} audiodata;
audiodata data;

int continuousNonSpeech = 0;
int firstNonSpeechIndex = 0;
int flag = 0;
int totalFrames;
float level;
float background;
float energyBuffer[10];

/* endpointing parameters */
const float forgetFactor = 1000;
const float thresholdRate = -0.35;
const float adjustment = 0.000005;

void operateError(PaError err)
{
	Pa_Terminate();
    printf("An error occured while using the portaudio stream\n" );
    printf("Error number: %d\n", err );
    printf("Error message: %s\n", Pa_GetErrorText( err ) );
	exit(1);
}

/* calculate the energy of a specific frame */
float calcEnergy(int index)
{
	int sampleNum = 5, start, end;
	float squareSum = 0;

	// get the range of samples that need to be summed up to calculate energy
	if (index < sampleNum / 2) {
		start = 0; 
		end = sampleNum;
	} else if (index >= data.recordedFrameNum - sampleNum / 2) {
		start = data.recordedFrameNum - sampleNum;
		end = data.recordedFrameNum;
	} else {
		start = index - sampleNum / 2;
		end = index + sampleNum / 2 + 1;
	}

	// calculate square sum
	for (int i = start; i < end; i++) {
		float temp = (float)data.recordedData[i] / 32768; // normalization
		squareSum += temp * temp;
	}

	if (squareSum == 0)
		squareSum = 0.000001;

	return 10 * log(squareSum);
}

/* calculate the initial value of background */
float calcInitBackground()
{
	float result = 0;
	int sampleNum = 10;
	for (int i = 0; i < sampleNum; i++) {
		energyBuffer[i] = calcEnergy(i);
		result += energyBuffer[i];
	}
	return result / sampleNum;
}

/* classify a specific frame */
int classifyFrame(int index)
{
	int isSpeech = 0;
	float energy = calcEnergy(index);
	level = (level * forgetFactor + energy) / (forgetFactor + 1);

	if (energy < background)
		background = energy;
	else
		background += (energy - background) * adjustment;

	if (level < background)
		level = background;

	if (index % 5000 == 0)
		printf("index: %d: %.8f %.8f\n", index, level, background);
	if (level - background > background * thresholdRate)
		isSpeech = 1;
	return isSpeech;
}

/* whether frame at frameIndex is speech */
bool dealWithFrame(int frameIndex)
{
	int isSpeech = classifyFrame(frameIndex);
	if (isSpeech) {
		if (continuousNonSpeech > 0.1 * sampleRate) 
			data.outputFrameNum = data.outputFrameIndex = firstNonSpeechIndex;
		flag = 0;
		continuousNonSpeech = 0;
	}
	else {
		if (flag == 0) {
			firstNonSpeechIndex = data.outputFrameIndex;
			flag = 1;
		}
		continuousNonSpeech++;
		if (continuousNonSpeech > 1 * sampleRate) {
			data.outputFrameNum = firstNonSpeechIndex;
			return false;
		}
	}
	data.outputData[data.outputFrameIndex++] = data.recordedData[frameIndex];
	data.outputFrameNum++;
	return true;
}

/* callback function doing endpointing */
static int cutFrameCallback(const void *inputBuffer, void *outputBuffer,
	          unsigned long framesPerBuffer,
			  const PaStreamCallbackTimeInfo* timeInfo,
			  PaStreamCallbackFlags statusFlags,
			  void *userData)
{
	const SAMPLE *input = (const SAMPLE*)inputBuffer;
	for (int i = 0; i < framesPerBuffer; i++) {
		data.recordedData[data.frameIndex + i] = input[i];
	}

	// initialize parameters
	data.recordedFrameNum += framesPerBuffer;
	if (data.frameIndex == 0) {
		background = calcInitBackground();
		level = energyBuffer[0];
	}

	// for each frame, see whether it is speech
	for (int i = 0; i < framesPerBuffer; i++, data.frameIndex++) 
		if (!dealWithFrame(data.frameIndex))
			return paComplete;
	
	return paContinue;
}

/* call back function playing the output audio */
static int playCallback(const void *inputBuffer, void *outputBuffer,
	          unsigned long framesPerBuffer,
			  const PaStreamCallbackTimeInfo* timeInfo,
			  PaStreamCallbackFlags statusFlags,
			  void *userData)
{
	SAMPLE *output = (SAMPLE *) outputBuffer;
	int framesToMove = framesPerBuffer;
	int finished;

	if (data.outputFrameNum < framesPerBuffer) { 
		framesToMove = data.outputFrameNum;
		finished = paComplete;
	} else {
		finished = paContinue;
	}

	for (int i = 0; i < framesToMove; i++) {
		output[i] = data.outputData[data.outputFrameIndex + i] * 4;
	}
	data.outputFrameIndex += framesToMove;
	data.outputFrameNum -= framesToMove;

	return finished;
}

void initAudioData()
{
	data.frameIndex = 0;
	data.outputFrameIndex = 0;
	data.recordedFrameNum = 0;
	data.outputFrameNum = 0;
	int maxDuration = 300;
	int maxFrameNum = sampleRate * maxDuration;
	int maxByteNum = maxFrameNum * sizeof(SAMPLE);
	data.recordedData = (SAMPLE *)malloc(maxByteNum);
	data.outputData = (SAMPLE *)malloc(maxByteNum);
}

void initPa()
{
	PaError err = paNoError;
	err = Pa_Initialize();
	if (err != paNoError)
		operateError(err);
}

void stopPa()
{
	PaError err = paNoError;
	err = Pa_Terminate();
	if (err != paNoError)
		operateError(err);
}

/* capture the audio data and call cutFrameCallback() to do endpointing */
void captureAndEndpointing()
{
	PaStream *stream;
	PaStreamParameters inputParameters;
	PaError err = paNoError;

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
		exit(1);
    }

	inputParameters.channelCount = 1;
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device) -> defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,
		sampleRate,
		framesPerBuffer,
		paClipOff,
		cutFrameCallback,
		NULL);

	if (err != paNoError) 
		operateError(err);

	// start stream
	err = Pa_StartStream(stream);
	if(err != paNoError)
		operateError(err);

	printf("\n=== Start recording ===\n");

	// empty loop to enable the callback function to be called
	while(Pa_IsStreamActive(stream) == 1) {}

	// close stream
	err = Pa_CloseStream(stream);
	if (err != paNoError)
		operateError(err);

	printf("recorded num: %d, left data: %d\n\n" , data.recordedFrameNum, data.outputFrameNum);
	totalFrames = data.outputFrameNum;
}

/* play the audio after endpointing */
void playAudio()
{
	data.outputFrameIndex = 0;

	PaStream *stream;
	PaStreamParameters outputParameters;
	PaError err = paNoError;

	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
		exit(1);
    }

	outputParameters.channelCount = 1;                     
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device) -> defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

	printf("\n=== Now playing back. ===\n");
	err = Pa_OpenStream(
		&stream,
		NULL,
		&outputParameters,
		sampleRate,
		framesPerBuffer,
		paClipOff,
		playCallback,
		NULL);
	if (err != paNoError) operateError(err);

	err = Pa_StartStream(stream);
    if(err != paNoError) operateError(err);
        
    printf("Waiting for playback to finish.\n");

	while(Pa_IsStreamActive(stream) == 1) {}
        
    err = Pa_CloseStream(stream);
    if(err != paNoError) operateError(err);

	printf("Done!\n\n");
}

void writeAudioToFile()
{
	//printf("\n=== Writing data into recordedData.wav ===\n");
	WriteWave("recordedData.wav", data.outputData, totalFrames, sampleRate);
	//printf("Done!\n\n");
}

void waitForPressKey() { printf("Press any key to record. \n");  while (!kbhit());}
