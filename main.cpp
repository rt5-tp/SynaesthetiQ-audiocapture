#include "AudioCapture.h"
#include <iostream>
#include <chrono>


//create buffer in main, pass same circ buffer to both, aggregation not inheritance
// make queue system, thead pull and queue

// audioCapture
// FFT
// 

int main(int argc, char* argv[]) {
    std::cout << "Initialising!" << std::endl;
    bool sdl_enabled = false;
    int device_index = 0;
    bool skipSearch = false;

    
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        // int iarg = stoi(arg);
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
        audioCapture.startCapture();
        std::cout << "test" << std::endl;
        // audioCapture.isCapturing();
        std::cout << "Waiting...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "Done.\n";
        audioCapture.stopCapture();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    free(name);
    //snd_device_name_free_hint (hints);

    std::cout << "Complete" << std::endl;
    return 0;
}
