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

class AudioCapture {
public:
    typedef void (*DataAvailableCallback)(const std::vector<short>&);

    AudioCapture(const std::string& device_name, bool sdl_enabled, PingPongBuffer& buffer);
    ~AudioCapture();

    //Callback test
    void register_callback(DataAvailableCallback cb);

    const std::vector<int>& get_buffer() const;

private:
    static void MyCallback(snd_async_handler_t* pcm_callback);
    void performFFT(const std::vector<short>& data);

    std::ofstream audioFile;
    snd_pcm_t* handle;
    snd_async_handler_t* pcm_callback;
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::vector<float> waveform;
    static bool quit;
    bool m_sdl_enabled;

    std::vector<short> fftInputData;

    std::vector<int> tempbuffer;
    DataAvailableCallback callback;

    PingPongBuffer& buffer_;
};

#endif  // AUDIO_CAPTURE_H
