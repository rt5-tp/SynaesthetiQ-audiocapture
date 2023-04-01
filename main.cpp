#include <iostream>
#include <SDL2/SDL.h>
#include <alsa/asoundlib.h>
#include <fstream>
#include <vector>
#include <climits>
#include <thread>
#include <string>
#include <signal.h>
#include <fftw3.h>
#include <cstring>

#include "AudioCapture.h"
#include "FFTProcessor.h"




class RawAudioWriter {
public:
    RawAudioWriter(const std::string& filename) : filename_(filename) {
        file_.open(filename_, std::ios::binary);
        if (!file_.is_open()) {
            throw std::runtime_error("Could not open file for writing.");
        }
    }

void WriteData(const std::vector<short>& buffer) {
    file_.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(short));
}

void WriteData(const std::vector<char>& buffer) {
    file_.write(buffer.data(), buffer.size());
}

    ~RawAudioWriter() {
        if (file_.is_open()) {
            file_.close();
        }
    }

private:
    std::string filename_;
    std::ofstream file_;
};


RawAudioWriter writer("audio.raw");
FFTProcessor fftProcessor; //create instance of fftprocessor class


std::vector<char> convertRawAudioToWav(std::vector<int16_t> rawAudioData)
{
    std::vector<char> outBuffer;
    // Write WAV header to output buffer
    const int16_t numChannels = 1;
    const int sampleRate = 44100;
    const int numSamples = rawAudioData.size();
    const int32_t subchunk1Size = 16;
    const int16_t audioFormat = 1;
    const int32_t byteRate = sampleRate * sizeof(int16_t) * numChannels;
    const int16_t blockAlign = sizeof(int16_t) * numChannels;
    const int16_t blob = sizeof(int16_t) * 8;
    const int32_t subchunk2Size = numSamples * sizeof(int16_t) * numChannels;
    const int32_t chunkSize = subchunk2Size + 44;

    outBuffer.resize(chunkSize);

    std::memcpy(&outBuffer[0], "RIFF", 4);
    std::memcpy(&outBuffer[4], &chunkSize, 4);
    std::memcpy(&outBuffer[8], "WAVE", 4);
    std::memcpy(&outBuffer[12], "fmt ", 4);
    std::memcpy(&outBuffer[16], &subchunk1Size, 4);
    std::memcpy(&outBuffer[20], &audioFormat, 2);
    std::memcpy(&outBuffer[22], &numChannels, 2);
    std::memcpy(&outBuffer[24], &sampleRate, 4);
    std::memcpy(&outBuffer[28], &byteRate, 4);
    std::memcpy(&outBuffer[32], &blockAlign, 2);
    std::memcpy(&outBuffer[34], &blob, 2);
    std::memcpy(&outBuffer[36], "data", 4);
    std::memcpy(&outBuffer[40], &subchunk2Size, 4);

    
    // Write raw audio data to output buffer
    std::memcpy(&outBuffer[44], rawAudioData.data(), numSamples * sizeof(int16_t) * numChannels);
    return outBuffer;
}


// record a few seconds of audio
std::vector<int16_t> rec_audio;

void data_available_callback(const std::vector<short>& data) {

    // append to vector
    rec_audio.insert(rec_audio.end(), data.begin(), data.end());
    writer.WriteData(data);
    fftProcessor.processData(data);

    // Callback function for the AudioCapture class, currenlty unused
    std::cout << "Data available: " << data.size() << " samples" << std::endl;
}

void on_buffer_full(const std::vector<short>& full_buffer, int buffer_index)
{
    // Do something with the full buffer, such as writing it to a file (i.e the writer class)
    writer.WriteData(full_buffer);
    fftProcessor.processData(full_buffer);

    // std::cout << "Hello from buffer " << (buffer_index == 0 ? "A" : "B") << " callback" << std::endl;
    std::cout << "Buffer callback data size = " << full_buffer.size() << std::endl;
}



// FFT callback function
void onFFTDataAvailable(const std::vector<double> &data) {
    // This is the FFT output data for further processing
    std::cout << "FFT data available!" << std::endl;
    std::cout << "FFT data size = " << data.size() << std::endl;
}



RawAudioWriter wavWriter("wavy.wav");


int main(int argc, char* argv[]) {
    std::cout << "Initialising!" << std::endl;
    bool sdl_enabled = false;
    int device_index = 0;
    bool skipSearch = false;

    
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        int iarg = atoi(arg.c_str());
        if (arg == "sdl") {
            sdl_enabled = true;
        }
        else if (iarg != 0) {
            device_index = iarg;
            skipSearch = true;
        }
        else if (arg == "null") {
            skipSearch = true;
        }      
    }


    // Get a list of available audio devices
    void **hints;
    if (snd_device_name_hint(-1, "pcm", &hints) != 0) {
        std::cerr << "Error getting audio device hints" << std::endl;
        return 1;
    }
    int i = 0;

    if (!skipSearch) {
        // Print the list of available audio devices

        for (void **hint = hints; *hint; hint++) {
            char *name = snd_device_name_get_hint(*hint, "NAME");
            char *desc = snd_device_name_get_hint(*hint, "DESC");
            std::cout << i++ << ". " << name << " - " << desc << std::endl;
            free(name);
            free(desc);
        }

        // Prompt the user to select an audio device
        
        std::cout << "Enter the index of the audio device to use: ";
        std::cin >> device_index;
    }



    // Get the name of the selected audio device
    i = 0;
    char *name;

    for (void **hint = hints; *hint; hint++) {
        if (i++ == device_index) {
            name = snd_device_name_get_hint(*hint, "NAME");
            //device_name = name;
            //free(name);
            break;
        }
    }

    // Free the memory used by the device hints
    snd_device_name_free_hint(hints);

    try {
        
        AudioCapture audioCapture(name, sdl_enabled);
        audioCapture.register_callback(&data_available_callback);

        //update callbacks for consistency
        fftProcessor.registerCallback(onFFTDataAvailable);



        std::cout << "Starting" << std::endl;
        // audioCapture.isCapturing();
        std::cout << "Waiting...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "Done.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    std::vector<char> wavData = convertRawAudioToWav(rec_audio);
    wavWriter.WriteData(wavData);

    free(name);
    //snd_device_name_free_hint (hints);

    std::cout << "Complete" << std::endl;
    return 0;
}
