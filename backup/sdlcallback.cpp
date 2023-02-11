#include <iostream>
#include <alsa/asoundlib.h>
#include <fstream>
#include <SDL2/SDL.h>


std::ofstream audioFile("audio.raw", std::ios::binary);
SDL_Renderer* renderer;


void MyCallback(snd_async_handler_t *pcm_callback)
{
    // printf("test");
    snd_pcm_t *handle = snd_async_handler_get_pcm(pcm_callback);
    snd_pcm_sframes_t avail = snd_pcm_avail_update(handle);
    if (avail < 0)
    {
        std::cerr << "Error in snd_pcm_avail_update: " << snd_strerror(avail) << std::endl;
        return;
    }

    if (avail > 250)
    {
        std::cout << "Too big  " << std::endl;
        // return;
    }
    std::cout << "Avail = " << avail << std::endl;
    
    static char buffer[4096];

    
    snd_pcm_sframes_t frames = snd_pcm_readi(handle, buffer, avail);


    std::cout << "frames = " << frames << std::endl;
    if (frames < 0)
    {
        std::cerr << "Error in snd_pcm_readi: " << snd_strerror(frames) << std::endl;
        return;
    }
    // Process the captured audio data in 'buffer'
    audioFile.write((char*) buffer, avail * sizeof(short));
    // int buffer_size = 4096;
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Set the color for the waveform
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    // Calculate the vertical scale factor to fit the waveform in the window
    float scale_factor = (float)600 / ((float)32768/4);
        
    int num_samples = avail / sizeof(int16_t);;
    std::cout << num_samples << std::endl;

    // Draw the waveform
    for (int i = 0; i < num_samples - 1; i++) {
        int x1 = (int)((float)i / (float)num_samples * 800.0f);
        int y1 = (int)((float)buffer[i] * scale_factor + 300.0f);
        int x2 = (int)((float)(i + 1) / (float)num_samples * 800.0f);
        int y2 = (int)((float)buffer[i + 1] * scale_factor + 300.0f);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }  
    SDL_RenderPresent(renderer);

}

int main()
{
    snd_pcm_t *handle;

    
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL2: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create the SDL2 window and renderer
    SDL_Window* window = SDL_CreateWindow("Audio Waveform", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Error creating SDL2 window: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Error creating SDL2 renderer: " << SDL_GetError() << std::endl;
        return 1;
    }

    
    // can't use 'default' as it is a virtual device
    int err = snd_pcm_open(&handle, "plughw:CARD=webcam,DEV=0", SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0)
    {
        std::cerr << "Error opening PCM device: " << snd_strerror(err) << std::endl;
        return 1;
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


    snd_async_handler_t *pcm_callback;
    err = snd_async_add_pcm_handler(&pcm_callback, handle, MyCallback, NULL);
    if (err < 0)
    {   
        printf("yeet");
        std::cerr << "Error setting PCM async handler: " << snd_strerror(err) << std::endl;
        return 1;
    }

    err = snd_pcm_start(handle);
    if (err < 0)
    {
        std::cerr << "Error starting PCM device: " << snd_strerror(err) << std::endl;
        return 1;
    }




    while (true)
    {
        // Check for keyboard events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
            exit(0);
            } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_q) {
                exit(0);
            }
            }
        }
        // Wait for audio data to be captured and processed by the callback function
    }

   // Clean up
    // delete[] buffer;

    // Clean up SDL2
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    audioFile.close();
    snd_pcm_close(handle);
    return 0;
}