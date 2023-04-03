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
#include "GenreClassifier.h"


FFTProcessor fftProcessor; //create instance of fftprocessor class


void data_available_callback(const std::vector<short>& data) {

    fftProcessor.processData(data);

    // Callback function for the AudioCapture class, currenlty unused
    //std::cout << "Data available: " << data.size() << " samples" << std::endl;
}

void genre_prediction_callback(const std::vector<std::pair<std::string, float>>& predictions){
    std::cout << "\nGenre Predictions callback: \n";
    for(auto prediction : predictions){
        std::cout << prediction.first << " : " << prediction.second << std::endl;
    }
    std::cout << std::endl;
}

// FFT callback function
void onFFTDataAvailable(const std::vector<double> &data) {
    // This is the FFT output data for further processing
    //std::cout << "FFT data available!" << std::endl;
    //std::cout << "FFT data size = " << data.size() << std::endl;
}


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

    GenreClassifier classifier;

    // Free the memory used by the device hints
    snd_device_name_free_hint(hints);

    try {
        
        classifier.register_genre_callback(&genre_prediction_callback);

        AudioCapture audioCapture(name, sdl_enabled);
        audioCapture.register_callback(classifier.audio_callback);
        audioCapture.register_callback(&data_available_callback);

        //update callbacks for consistency
        fftProcessor.registerCallback(onFFTDataAvailable);

        std::cout << "Starting" << std::endl;
        std::cout << "Waiting...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "Done.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    

    free(name);
    //snd_device_name_free_hint (hints);

    std::cout << "Complete" << std::endl;
    return 0;
}

