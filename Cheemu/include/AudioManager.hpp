#pragma once
#include <SDL2/SDL.h>

class AudioManager {
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;

	char * buffer;
    int buf_time_ms = 100;
	int buf_size;
	float freq = 440.0;

    bool isPlaying = false;
    bool isFinished = false;

public:
    AudioManager();
    ~AudioManager();

    void loadSamples();
    void play();
};
