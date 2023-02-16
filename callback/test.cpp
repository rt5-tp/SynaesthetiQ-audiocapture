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
    AudioCapture() : audioFile("audio.raw", std::ios::binary) {
        int err = snd_pcm_open(&handle, "plughw:CARD=webcam,DEV=0", SND_PCM_STREAM_CAPTURE, 0);
        if (err < 0) {
            std::cerr << "Error opening PCM device: " << snd_strerror(err) << std::endl;
            throw std::runtime_error("Failed to open PCM device");
        }

        snd_pcm_hw_params_t *params;
        snd_pcm_hw_params_alloca(&params);
        snd_pcm_hw_params_any(handle, params);
        snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

        unsigned int sample_rate = 44100;
        snd_pcm_hw_params_set_rate_near(handle, params, &sample_rate, nullptr);
        snd_pcm_hw_params_set_channels(handle, params, 1);
        snd_pcm_hw_params(handle, params);

        err = snd_async_add_pcm_handler(&pcm_callback, handle, &AudioCapture::MyCallback, this);
        if (err < 0) {
            std::cerr << "Error setting PCM async handler: " << snd_strerror(err) << std::endl;
            throw std::runtime_error("Failed to set PCM async handler");
        }

        err = snd_pcm_start(handle);
        if (err < 0) {
            std::cerr << "Error starting PCM device: " << snd_strerror(err) << std::endl;
            throw std::runtime_error("Failed to start PCM device");
        }

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to initialize SDL");
        }

        // Create window and renderer
        window = SDL_CreateWindow("Audio Capture", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to create window");
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Error creating renderer: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to create renderer");
        }

        signal(SIGINT, AudioCapture::signalHandler);

        // Initialize waveform data
        waveform.reserve(800);

        
    }

    ~AudioCapture() {
        audioFile.close();
        snd_pcm_close(handle);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void capture() {
        printf("Initialising!");
        while (!quit) {
            // Wait for audio data to be captured and processed by the callback function
        }
    }

private:
    static void MyCallback(snd_async_handler_t *pcm_callback) {
        AudioCapture* audioCapture = reinterpret_cast<AudioCapture*>(snd_async_handler_get_callback_private(pcm_callback));
        snd_pcm_t *handle = snd_async_handler_get_pcm(pcm_callback);
        snd_pcm_sframes_t avail = snd_pcm_avail_update(handle);
        if (avail < 0) {
            std::cerr << "Error in snd_pcm_avail_update: " << snd_strerror(avail) << std::endl;
            return;
        }

        static char buffer[4096];
        snd_pcm_sframes_t frames = snd_pcm_readi(handle, buffer, avail);

        if (frames < 0) {
            std::cerr << "Error in snd_pcm_readi: " << snd_strerror(frames) << std::endl;
            return;
        }

        // Process the captured audio data in 'buffer'
        audioCapture->audioFile.write((char*) buffer, avail * sizeof(short));

        // Update waveform data
        audioCapture->waveform.clear();
        for (int i = 0; i < frames; ++i) {
            short sample = ((short*) buffer)[i];
            float sampleValue = sample / static_cast<float>(SHRT_MAX);
            audioCapture->waveform.push_back(sampleValue);
        }

        // Render waveform
        SDL_SetRenderDrawColor(audioCapture->renderer, 0, 0, 0, 255);
        SDL_RenderClear(audioCapture->renderer);
        SDL_SetRenderDrawColor(audioCapture->renderer, 255, 255, 255, 255);
        for (std::vector<float>::size_type i = 0; i < audioCapture->waveform.size() - 1; ++i) {
            int x1 = i * 800 / audioCapture->waveform.size();
            int y1 = (1 - audioCapture->waveform[i]) * 300 + 50;
            int x2 = (i + 1) * 800 / audioCapture->waveform.size();
            int y2 = (1 - audioCapture->waveform[i+1]) * 300 + 50;
            SDL_RenderDrawLine(audioCapture->renderer, x1, y1, x2, y2);
        }
        SDL_RenderPresent(audioCapture->renderer);
    }

    // Signal handler for Ctrl+C
    static void signalHandler(int signal) {
        std::cout << "Received signal " << signal << ". Exiting program." << std::endl;
        quit = true;

        // exit(signal);
    }

    std::ofstream audioFile;
    snd_pcm_t *handle;
    snd_async_handler_t *pcm_callback;
    SDL_Window *window;
    SDL_Renderer *renderer;
    std::vector<float> waveform;
    static bool quit;


};

bool AudioCapture::quit = false;



int main() {
    try {
        AudioCapture audioCapture;
        audioCapture.capture();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
    }
return 0;
}