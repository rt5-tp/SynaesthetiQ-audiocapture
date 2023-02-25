#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include <iostream>
#include <SDL2/SDL.h>
#include <alsa/asoundlib.h>
#include <fstream>
#include <vector>
#include <climits>
#include <thread>
#include <string>
#include <signal.h>

class AudioCapture {
public:
    AudioCapture(const std::string& device_name, bool sdl_enabled = false);

    ~AudioCapture();

    void capture();

private:
    static void MyCallback(snd_async_handler_t* handler);
    static void signalHandler(int signum);

    std::ofstream audioFile;
    snd_pcm_t* handle;
    snd_async_handler_t* pcm_callback;
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::vector<float> waveform;
    static bool quit;
};

#endif
