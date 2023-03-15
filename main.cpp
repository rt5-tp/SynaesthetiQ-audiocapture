// #include "tempAudioCapture.h"
// #include <iostream>
// #include <chrono>
// #include <vector>


//create buffer in main, pass same circ buffer to both, aggregation not inheritance
// make queue system, thead pull and queue

// audioCapture
// FFT
// 
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

#include "PingPongBuffer.h"
#include "AudioCapture.h"




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


    ~RawAudioWriter() {
        if (file_.is_open()) {
            file_.close();
        }
    }

private:
    std::string filename_;
    std::ofstream file_;
};


RawAudioWriter writer("audionew.raw");


void performFFT(const std::vector<short> &data)
{
    // Perform FFT operations on the copied data
    std::cout << "FFT function called!" << std::endl;

    int N = data.size();
    fftw_complex *in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < N; i++)
    {
        in[i][0] = data[i];
        in[i][1] = 0;
    }
    fftw_execute(p);
    fftw_destroy_plan(p);
    fftw_free(in);

    // Open file for writing
    std::ofstream outfile;
    outfile.open("fft_output.txt");

    // Write FFT output data to file
    for (int i = 0; i < N; i++)
    {
        outfile << out[i][0] << "," << out[i][1] << "\n";
    }

    outfile.close(); // Close file

    // Calculate the magnitude spectrum of the FFT output
    double *mag_spectrum = new double[N / 2];
    for (int i = 0; i < N / 2; i++)
    {
        mag_spectrum[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
    }

    // Find the index of the maximum value in the magnitude spectrum
    int max_idx = 0;
    double max_val = mag_spectrum[0];
    for (int i = 1; i < N / 2; i++)
    {
        if (mag_spectrum[i] > max_val)
        {
            max_idx = i;
            max_val = mag_spectrum[i];
        }
    }

    // Convert the index to a frequency value
    double Fs = 44100; // Replace with the actual sampling rate
    double freq = (double)max_idx / N * Fs;

    // Free memory
    delete[] mag_spectrum;
    fftw_free(out);

    std::cout << "Most prominent frequency: " << freq << " Hz" << std::endl;
    // std::cout << "Done." << std::endl;
}


void data_available_callback(const std::vector<short>& data) {
    // Use the data vector here
    std::cout << "Data available: " << data.size() << " samples" << std::endl;
}

void on_buffer_full(const std::vector<short>& full_buffer, int buffer_index)
{
    // Do something with the full buffer, such as writing it to a file
    writer.WriteData(full_buffer);
    performFFT(full_buffer);
    // std::cout << "New function test" << std::endl;

    // std::cout << "Hello from buffer " << (buffer_index == 0 ? "A" : "B") << " callback" << std::endl;
    // std::cout << "Callback data size = " << full_buffer.size() << std::endl;
}








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
        PingPongBuffer buffer(4096);
        // Set up the callback functions to be called when buffer A or B is full
        buffer.set_on_buffer_full_callback(on_buffer_full);


        AudioCapture audioCapture(name, sdl_enabled, buffer);
        audioCapture.register_callback(&data_available_callback);

        

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
