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
#include <fftw3.h>

#include "PingPongBuffer.h"

// A wrapper around the asound library
class AudioCapture {
public:
    typedef void (*DataAvailableCallback)(const std::vector<short>&);
    /**
     * @brief Constructor for this class
     *
     * Prompts user for audio device if not provided. 
     * Gets handle for audio device and sets up necessary callbacks.
     */
    AudioCapture(std::string device_name);

    /**
     * @brief Registers callback for audio data
     *
     * Calls to this function is equivalent to subscribing to the data.
     * Multiple subscribers can be set up by making multiple calls.
     */
    void register_callback(DataAvailableCallback cb);

    /**
     * @brief Prompts user to select audio device
     *
     * Lists all discoverable audio devices on system and prompts user to input a number selection.
     */
    std::string prompt_device_selection();

    ~AudioCapture();

private:

    /**
     * @brief Callback function used to handle audio originating from hardware 
     *
     * This function reads the available audio data into a PingPongBuffer.
     * Optionally, it renders a visualisation of the audio data.
     */
    static void MyCallback(snd_async_handler_t* pcm_callback);

    /**
     * @brief Calls all callback functions with audio data when PingPongBuffer is full
     *
     * This function reads iterates through all callbacks registered with 'register_callback'.
     * Passes audio data from the PingPongBuffer, which is typically 4096 bytes.
     */
    static void call_callbacks(const std::vector<short>& full_buffer, int);

    std::ofstream audioFile;
    snd_pcm_t* handle;
    snd_async_handler_t* pcm_callback;
    std::vector<float> waveform;
    static std::vector<DataAvailableCallback> callbacks;
    static bool quit;

    std::vector<short> fftInputData;

    std::vector<int> tempbuffer;
    DataAvailableCallback callback;

    PingPongBuffer buffer_;
};

#endif  // AUDIO_CAPTURE_H
