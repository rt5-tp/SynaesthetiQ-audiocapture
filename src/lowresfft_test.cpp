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

std::vector<std::vector<bool>> convertFFTToLEDMatrix(const std::vector<double> &fftData, int rows, int cols, double maxFrequency)
{
    std::vector<std::vector<bool>> ledMatrix(rows, std::vector<bool>(cols, false));

    double maxAmplitude = *std::max_element(fftData.begin(), fftData.end());

    double minFrequencyLog = std::log10(1);
    double maxFrequencyLog = std::log10(maxFrequency);
    double logRange = maxFrequencyLog - minFrequencyLog;

    double cutoffThreshold = 1; //* maxAmplitude; // You can adjust this value to your preference
    std::cout << "Cutoff = " << cutoffThreshold << std::endl;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            double targetFrequencyLog = minFrequencyLog + (j * logRange) / cols;
            double targetFrequency = std::pow(10, targetFrequencyLog);
            double lowerBound = targetFrequency - targetFrequency / 2;
            double upperBound = targetFrequency + targetFrequency / 2;

            double maxValueInBin = 0;
            int lowerIndex = static_cast<int>(lowerBound / maxFrequency * fftData.size());
            int upperIndex = std::min(static_cast<int>(upperBound / maxFrequency * fftData.size()), static_cast<int>(fftData.size()));

            for (int k = lowerIndex; k < upperIndex; ++k)
            {
                maxValueInBin = std::max(maxValueInBin, fftData[k]);
            }
             std::cout << "max value in bin = " << maxValueInBin << std::endl;

            if (maxValueInBin > cutoffThreshold)
            {
                double normalizedAmplitude = maxValueInBin / maxAmplitude;
                std::cout << "Normalized Amplitude at bin " << j << ": " << normalizedAmplitude << std::endl;
                int threshold = static_cast<int>(normalizedAmplitude * rows);
                if (i < threshold)
                {
                    ledMatrix[rows - i - 1][j] = true;
                }
            } else {std::cout << "TOO SMALL" << std::endl;}
        }
    }

    return ledMatrix;
}


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
    std::cout << "FFT DATA AVAILABLE" << std::endl;
    int rows = 16;
    int cols = 32;
    double maxFrequency = 16000;
    std::vector<std::vector<bool>> ledMatrix = convertFFTToLEDMatrix(data, rows, cols, maxFrequency);
    // printLEDMatrix(ledMatrix);

    // Output the LED matrix to your physical display
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

int main(int argc, char *argv[]) {
    std::cout << "Initialising!" << std::endl;
    bool sdl_enabled = false;
    std::string device_name = "";

    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        int iarg = atoi(arg.c_str());
        if (arg == "sdl") {
            sdl_enabled = true;
        } else if (iarg != 0) {
            device_name = arg;
        }
    }

    GenreClassifier classifier;

    try {
        classifier.register_genre_callback(&genre_prediction_callback);

        AudioCapture audioCapture(device_name, sdl_enabled);
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

        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
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

            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    if (ledMatrixCopy[i][j]) {
                        SDL_Rect rect;
                        rect.x = j * cellWidth;
                        rect.y = i * cellHeight;
                        rect.w = cellWidth;
                        rect.h = cellHeight;

                        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
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
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    std::cout << "Complete" << std::endl;
    return 0;
}