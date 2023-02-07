#include <iostream>
#include <alsa/asoundlib.h>
#include <fstream>


std::ofstream audioFile("audio.raw", std::ios::binary);

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
        return;
    }
    std::cout << "Avail = " << avail << std::endl;
    
    static char buffer[4096];
    
    // // Allocate memory for audio data
    // const int buffer_size = 1024; //reduce this
    // short* buffer = new short[buffer_size];
    
    snd_pcm_sframes_t frames = snd_pcm_readi(handle, buffer, avail);


    std::cout << "frames = " << frames << std::endl;
    if (frames < 0)
    {
        std::cerr << "Error in snd_pcm_readi: " << snd_strerror(frames) << std::endl;
        return;
    }
    // Process the captured audio data in 'buffer'
    audioFile.write((char*) buffer, avail * sizeof(short));
}

int main()
{
    snd_pcm_t *handle;
    int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
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
        // Wait for audio data to be captured and processed by the callback function
    }

    snd_pcm_close(handle);
    return 0;
}