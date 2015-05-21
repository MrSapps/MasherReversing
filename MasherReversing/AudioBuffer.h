#pragma once

#include <SDL.h>
#include <iostream>
#include <vector>
#include <mutex>

typedef struct
{
	uint16_t left;
	uint16_t right;
} StereoStream;

// Hacky audio class to play data from masher.
class AudioBuffer
{
public:
	AudioBuffer();
	~AudioBuffer();

	void Init();

	// Sample data must be 16 bit pcm.
	void SendSamples(char * sampleData, int size);

	// Internal use
	long long mSampleTick = 0;
	std::vector<char> audioBuffer;
	std::mutex audioBufferMutex;
};

