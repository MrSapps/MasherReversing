#include "AudioBuffer.h"

#define SDL_BUFFER_SIZE 1024

AudioBuffer::AudioBuffer()
{
}


AudioBuffer::~AudioBuffer()
{
}

void AudioCallback(void *udata, Uint8 *stream, int len)
{
	AudioBuffer * buffer = (AudioBuffer*)udata;
	StereoStream * stereoStream = (StereoStream *)stream;
	int stereoStreamCount = len / sizeof(StereoStream);

	memset(stream, 0, len);

	buffer->audioBufferMutex.lock();

	int currentBufferSampleSize = buffer->audioBuffer.size() / 4;

	if (currentBufferSampleSize >= SDL_BUFFER_SIZE)
	{
		memcpy(stream, buffer->audioBuffer.data(), SDL_BUFFER_SIZE * 4);
		buffer->audioBuffer.erase(buffer->audioBuffer.begin(), buffer->audioBuffer.begin() + (SDL_BUFFER_SIZE * 4));
	}

	buffer->audioBufferMutex.unlock();
}

void AudioBuffer::Init()
{
	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec audioSpec;
	audioSpec.callback = AudioCallback;
	audioSpec.userdata = this;
	audioSpec.channels = 2;
    audioSpec.freq = 37800;
	audioSpec.samples = SDL_BUFFER_SIZE;
	audioSpec.format = AUDIO_S16;

	/* Open the audio device */
	if (SDL_OpenAudio(&audioSpec, NULL) < 0){
		fprintf(stderr, "Failed to initialize audio: %s\n", SDL_GetError());
		exit(-1);
	}

	SDL_PauseAudio(0);
}

void AudioBuffer::SendSamples(char * sampleData, int size)
{
	audioBufferMutex.lock();

	int lastSize = audioBuffer.size();
	audioBuffer.resize(lastSize + size);
	memcpy(audioBuffer.data() + lastSize, sampleData, size);

	audioBufferMutex.unlock();
}