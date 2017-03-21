#include "AudioManager.hpp"

#include <string.h>
#include <math.h>
#include <stdio.h>

AudioManager::AudioManager() {
    memset( &want, 0, sizeof(want) );

    want.freq = 48000;
    want.format = AUDIO_S16LSB;
    want.channels = 2;
    want.samples = 4096;
    want.callback = nullptr;

    dev = SDL_OpenAudioDevice( nullptr, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE );

    if( !dev ) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        exit( -1 );
    } else if( have.format != want.format ) {
        SDL_Log( "Didn't get desired audio format.\n" );
        exit( -1 );
    }

    // allocate fixed-size buffer in advance
    buf_size = 16/8 * want.channels * want.freq * buf_time_ms / 1000;
    buffer = new char[buf_size];

    // unpause the audio
    SDL_PauseAudioDevice( dev, 0 );
}

AudioManager::~AudioManager() {
    SDL_PauseAudioDevice( dev, 1 );
    delete [] buffer;
    SDL_CloseAudioDevice( dev );
}

void AudioManager::loadSamples() {
    // f(i) = A * sin( 2pi * f * i )
    // Amplitude: 32768 * 0.75
    // 32768 is highest # representable by signed 16 bits
    // 0.75 represents volume?
    // format.rate samples makes up 1 Hz

    int sample;

    for( int i = 0; i < want.freq * buf_time_ms / 1000; i++ ) {
        // sin wave
        // sample = (int)(0.5 * 32768.0 * sin(2 * M_PI * freq * ((float) i/format.rate)));

        // square wave
        /*
           if( sin( 2 * M_PI * freq * ((float) i/format.rate) ) > 0 ) {
           sample = (int)(0.5 * 32768.0);
           } else {
           sample = (int)(-0.5 * 32768.0);
           }
        */

        // saw wave
        sample = (int)(0.2 * 32768.0 *
                ( 2 *
                  ( freq * ((float) i/want.freq) -
                    floor( freq * ((float) i/want.freq)) )
                  - 1 ) );

        /* Put the same stuff in left and right channel */
        // i and i+1 make up one sample, each sample is 16 bits, char arr is 8 bits
        // per entry
        buffer[4*i] = buffer[4*i+2] = sample & 0xff;
        buffer[4*i+1] = buffer[4*i+3] = (sample >> 8) & 0xff;
    }
}

// TODO
// Do audio properly with a ring buffer
// Callback may be better than Queue
void AudioManager::play() {
    // makes sure we don't queue too much audio
    if( SDL_GetQueuedAudioSize( dev ) < buf_size ) {
        SDL_QueueAudio( dev, buffer, buf_size );
    }
}

