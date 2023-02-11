#include <iostream>
#include <alsa/asoundlib.h>
#include <fstream>


// Define a class to handle resource acquisition and release
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
    }

    ~AudioCapture() {
        audioFile.close();
        snd_pcm_close(handle);
    }

    void capture() {
        while (true) {
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
}

std::ofstream audioFile;
snd_pcm_t *handle;
snd_async_handler_t *pcm_callback;

};

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