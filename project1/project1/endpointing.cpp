#include <stdio.h>
#include "portaudio.h"

const double sampleRate = 16000;
const unsigned long framesPerBuffer = 128;

void operateError(PaError err)
{
	Pa_Terminate();
    printf("An error occured while using the portaudio stream\n" );
    printf("Error number: %d\n", err );
    printf("Error message: %s\n", Pa_GetErrorText( err ) );
}

static int callBack(const void *inputBuffer, void *outputBuffer,
	          unsigned long framesPerBuffer,
			  const PaStreamCallbackTimeInfo* timeInfo,
			  PaStreamCallbackFlags statusFlags,
			  void *userData)
{
	const float *input = (const float*)inputBuffer; 
	printf("aaaaaaaaaaaa");
	for (int i = 0; i < framesPerBuffer; i++) {
		printf("value: %.8f\n", *input++);
	}
	return 0;
}

bool startCapture()
{
	PaStream *stream;
	PaStreamParameters inputParameters;
	PaError err = paNoError;

	int deviceIndex = Pa_GetDefaultInputDevice();

	inputParameters.device = deviceIndex;
	inputParameters.channelCount = 2;
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device) -> defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,
		sampleRate,
		framesPerBuffer,
		paClipOff,
		callBack,
		NULL);

	if (err != paNoError) {
		operateError(err);
		return false;
	}

	err = Pa_StartStream( stream );
	if( err != paNoError ) {
		operateError(err);
		return false;
	}

	printf("Start recording!!!!!!!!");

	while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) {}

	return true;
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

int main()
{
	initPa();
	startCapture();
	stopPa();
	return 0;
}