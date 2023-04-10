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

#include <mutex> //temp
std::vector<std::vector<bool>> sharedLedMatrix;
std::mutex mtx;

FFTProcessor fftProcessor; // create instance of fftprocessor class

void data_available_callback(const std::vector<short> &data)
{

    fftProcessor.processData(data);

    // Callback function for the AudioCapture class, currenlty unused
    // std::cout << "Data available: " << data.size() << " samples" << std::endl;
}

void genre_prediction_callback(const std::vector<std::pair<std::string, float>> &predictions)
{
    std::cout << "\nGenre Predictions callback: \n";
    for (auto prediction : predictions)
    {
        std::cout << prediction.first << " : " << prediction.second << std::endl;
    }
    std::cout << std::endl;
}

/**
 * This function takes in FFT data, number of rows, number of columns, and a max frequency, and outputs a 2D matrix of bools.
 * @param fftData a vector of double values representing the FFT data
 * @param rows an integer representing the number of rows in the output LED matrix
 * @param cols an integer representing the number of columns in the output LED matrix
 * @param minFrequency a double representing the minimum frequency of the LED matrix display
 * @param maxFrequency a double representing the maximum frequency of the LED matrix display
 * @return a 2D vector of bool values representing the LED matrix display
 */
std::vector<std::vector<bool>> convertFFTToLEDMatrix(const std::vector<double> &fftData, int rows, int cols, double minFrequency, double maxFrequency)
{
    // Creates a 2D matrix of bools with size rows x cols, initialized to false
    std::vector<std::vector<bool>> ledMatrix(rows, std::vector<bool>(cols, false));

    // Finds the maximum amplitude in the FFT data
    double maxAmplitude = *std::max_element(fftData.begin(), fftData.end());

    // Sets the logarithmic frequency range of the LED matrix display
    double minFrequencyLog = std::log10(minFrequency);             // Set the minimum frequency cutoff to 30 Hz
    double maxFrequencyLog = std::log10(maxFrequency);   // Set the maximum frequency cutoff to the input max frequency
    double logRange = maxFrequencyLog - minFrequencyLog; // Calculate the logarithmic range of the frequency scale

    // Sets a cutoff threshold to ignore values below a certain amount
    double cutoffThreshold = 1;                               // If the amplitude value is less than this, don't show it on the LED matrix
    // std::cout << "Cutoff = " << cutoffThreshold << std::endl; // Output the cutoff threshold value for debugging

    // Calculates the scaling factor for the amplitudes based on the number of rows in the LED matrix display
    double yScaleFactor = static_cast<double>(rows) / maxAmplitude;

    // Loops through each row and column in the LED matrix display
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            // Calculates the center frequency of the current column
            double targetFrequencyLog = minFrequencyLog + (j * logRange) / cols; // Calculate the logarithmic frequency of the center of the current column
            double targetFrequency = std::pow(10, targetFrequencyLog);           // Convert the logarithmic frequency to linear frequency
            double lowerBound = targetFrequency - targetFrequency / 2;           // Calculate the lower bound of the target frequency range
            double upperBound = targetFrequency + targetFrequency / 2;           // Calculate the upper bound of the target frequency range

            // Searches for the maximum amplitude value in the target frequency range
            double maxValueInBin = 0;
            int lowerIndex = static_cast<int>(lowerBound / maxFrequency * fftData.size()); // Find the lower index valuee in the bin
            int upperIndex = std::min(static_cast<int>(upperBound / maxFrequency * fftData.size()), static_cast<int>(fftData.size())); // Find the upper index value in the bin
            for (int k = lowerIndex; k < upperIndex; k++)
            {
                maxValueInBin = std::max(maxValueInBin, fftData[k]);
            }
            // std::cout << "max value in bin = " << maxValueInBin << std::endl; // Output the maximum amplitude value in the current bin for debugging

            // If the maximum amplitude value is above the cutoff threshold, set the corresponding LED matrix element to true
            if (maxValueInBin > cutoffThreshold)
            {
                double scaledAmplitude = maxValueInBin * yScaleFactor; // Scale the amplitude value based on the number of rows in the LED matrix display
                // std::cout << "Scaled Amplitude at bin " << j << ": " << scaledAmplitude << std::endl; // Output the scaled amplitude value for debugging
                int threshold = static_cast<int>(scaledAmplitude);
                if (i < threshold)
                {
                    ledMatrix[rows - i - 1][j] = true; // If the current row index is below the scaled amplitude value, set the corresponding LED matrix element to true
                }
            }
        }
    }

    return ledMatrix; // Return the final LED matrix display
}

// Temp - can remove
void printLEDMatrix(const std::vector<std::vector<bool>> &ledMatrix)
{
    int rows = ledMatrix.size();
    int cols = ledMatrix[0].size();

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            std::cout << (ledMatrix[i][j] ? "#" : " ");
        }
        std::cout << std::endl;
    }
}

// FFT callback function
void onFFTDataAvailable(const std::vector<double> &data)
{
    // std::cout << "FFT DATA AVAILABLE" << std::endl;
    int rows = 16;
    int cols = 32;
    double minFrequency = 60; // Set minimum frequency for visualisation
    double maxFrequency = 16000; // Set maximum frequency for visualisation
    std::vector<std::vector<bool>> ledMatrix = convertFFTToLEDMatrix(data, rows, cols, minFrequency, maxFrequency); // 2D boolean vector to represent LED matrix
    // printLEDMatrix(ledMatrix); // Console printing for debugging 

    // Output the LED matrix to display here!

    // Temporary SDL functions
    mtx.lock();
    sharedLedMatrix = ledMatrix;
    mtx.unlock();
}

// int main(int argc, char *argv[])
// {
//     std::cout << "Initialising!" << std::endl;
//     bool sdl_enabled = false;
//     std::string device_name = "";

//     for (int i = 0; i < argc; i++)
//     {
//         std::string arg = argv[i];
//         int iarg = atoi(arg.c_str());
//         if (arg == "sdl")
//         {
//             sdl_enabled = true;
//         }
//         else if (iarg != 0)
//         {
//             device_name = arg;
//         }
//     }

//     GenreClassifier classifier;

//     try
//     {

//         classifier.register_genre_callback(&genre_prediction_callback);

//         AudioCapture audioCapture(device_name, sdl_enabled);
//         audioCapture.register_callback(classifier.audio_callback);
//         audioCapture.register_callback(&data_available_callback);

//         // update callbacks for consistency
//         fftProcessor.registerCallback(onFFTDataAvailable);

//         std::cout << "Starting" << std::endl;
//         std::cout << "Waiting...\n";
//         std::this_thread::sleep_for(std::chrono::seconds(5));
//         std::cout << "Done.\n";
//     }
//     catch (const std::exception &ex)
//     {
//         std::cerr << "Error: " << ex.what() << std::endl;
//         return 1;
//     }

//     std::cout << "Complete" << std::endl;
//     return 0;
// }


// Temporary main function for SDL visualisation 
int main(int argc, char *argv[])
{
    std::cout << "Initialising!" << std::endl;
    std::string device_name = "";



    GenreClassifier classifier;

    try
    {
        classifier.register_genre_callback(&genre_prediction_callback);

        AudioCapture audioCapture(device_name);
        audioCapture.register_callback(classifier.audio_callback);
        audioCapture.register_callback(&data_available_callback);

        // Update callbacks for consistency
        fftProcessor.registerCallback(onFFTDataAvailable);

        // Initialize SDL
        SDL_Init(SDL_INIT_VIDEO);

        SDL_Window *window = SDL_CreateWindow("LED Matrix Visualization",
                                              SDL_WINDOWPOS_CENTERED,
                                              SDL_WINDOWPOS_CENTERED,
                                              640, 480,
                                              SDL_WINDOW_SHOWN);

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        std::cout << "Starting" << std::endl;
        std::cout << "Waiting...\n";

        bool running = true;
        SDL_Event event;

        while (running)
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    running = false;
                }
            }

            mtx.lock();
            auto ledMatrixCopy = sharedLedMatrix;
            mtx.unlock();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            int rows = ledMatrixCopy.size();
            int cols = ledMatrixCopy[0].size();

            int cellWidth = 640 / cols;
            int cellHeight = 480 / rows;

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    if (ledMatrixCopy[i][j])
                    {
                        SDL_Rect rect;
                        rect.x = j * cellWidth;
                        rect.y = i * cellHeight;
                        rect.w = cellWidth;
                        rect.h = cellHeight;

                        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }

            SDL_RenderPresent(renderer);
            SDL_Delay(1000 / 60); // 60 FPS
        }

        // Stop processing the audio
        // ...

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        std::cout << "Done.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    std::cout << "Complete" << std::endl;
    return 0;
}