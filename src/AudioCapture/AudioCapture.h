#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <cmath>
#include <signal.h>
#include <climits>

#include <alsa/asoundlib.h>
#include <SDL2/SDL.h>
#include <fftw3.h>

#include "PingPongBuffer.h"

// A wrapper around the asound library, with a few extra bells and whistles (sdl)
class AudioCapture {
public:
    typedef void (*DataAvailableCallback)(const std::vector<short>&);
    AudioCapture(std::string device_name, bool sdl_enabled);

    // calls to this function is equivalent to subscribing to the data
    // multiple subscribers can be set up by making multiple calls
    void register_callback(DataAvailableCallback cb);

    std::string prompt_device_selection();

    ~AudioCapture();

private:
    static void MyCallback(snd_async_handler_t* pcm_callback);
    static void call_callbacks(const std::vector<short>& full_buffer, int);
    void performFFT(const std::vector<short>& data);

    std::ofstream audioFile;
    snd_pcm_t* handle;
    snd_async_handler_t* pcm_callback;
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::vector<float> waveform;
    static std::vector<DataAvailableCallback> callbacks;
    static bool quit;
    bool m_sdl_enabled;

    std::vector<short> fftInputData;

    std::vector<int> tempbuffer;
    DataAvailableCallback callback;

    PingPongBuffer buffer_;
};

#endif  // AUDIO_CAPTURE_H
