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
#include <cmath>
#include <algorithm>


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
    double sampleRate = 44100;
    int numRows = 16;
    int numCols = 32;

    // Limit to 20 kHz
    double maxFrequency = 10000;
    int maxIndex = static_cast<int>(data.size() * maxFrequency / (sampleRate / 2));

    // Normalize FFT data
    double maxValue = *std::max_element(data.begin(), data.begin() + maxIndex);
    std::vector<double> normalizedData(data.size());
    for (int i = 0; i < maxIndex; ++i) {
        normalizedData[i] = data[i] / maxValue;
    }

    // Calculate the bins
    std::vector<int> bins(numCols + 1);
    for (int i = 0; i <= numCols; ++i) {
        bins[i] = (maxIndex * i) / numCols;
    }

    // Create the 16x32 matrix
    std::vector<std::vector<double>> matrix(numRows, std::vector<double>(numCols, 0));

    // Fill the matrix with the max values for each bin
    for (int col = 0; col < numCols; ++col) {
        double maxValueInBin = 0;
        for (int i = bins[col]; i < bins[col + 1]; ++i) {
            maxValueInBin = std::max(maxValueInBin, normalizedData[i]);
        }
        int row = static_cast<int>((numRows - 1) * (1 - maxValueInBin));
        for (int r = 0; r <= row; ++r) {
            matrix[r][col] = 0;
        }
        for (int r = row + 1; r < numRows; ++r) {
            matrix[r][col] = maxValueInBin;
        }
    }

    // Print the matrix
    // for (int row = 0; row < numRows; ++row) {
    //     for (int col = 0; col < numCols; ++col) {
    //         std::cout << matrix[row][col] << " ";
    //     }
    //     std::cout << std::endl;
    // }


    // Save the matrix data to a file
    std::ofstream matrixFile("matrix_data.txt", std::ios::app);
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            matrixFile << matrix[row][col] << " ";
        }
        matrixFile << std::endl;
    }
    matrixFile << "---" << std::endl;
    matrixFile.close();

}


int main(int argc, char* argv[]) {
    std::cout << "Initialising!" << std::endl;
    bool sdl_enabled = false;
    std::string device_name = "";
    
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        int iarg = atoi(arg.c_str());
        if (arg == "sdl") {
            sdl_enabled = true;
        }
        else if (iarg != 0) {
            device_name = arg;
        }
    }

    GenreClassifier classifier;

    try {
        
        classifier.register_genre_callback(&genre_prediction_callback);

        AudioCapture audioCapture(device_name, sdl_enabled);
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

    std::cout << "Complete" << std::endl;
    return 0;
}

