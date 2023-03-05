#include <iostream>
#include <fstream>
#include <cmath>
#include <fftw3.h>

using namespace std;

int main() {
    // Open the input file
    ifstream file("audio.raw", ios::binary);

    // Get the size of the file
    file.seekg(0, ios::end);
    int file_size = file.tellg();
    file.seekg(0, ios::beg);

    // Calculate the number of chunks in the file
    int chunk_size = 4096;
    int num_chunks = file_size / chunk_size;

    // Allocate memory for the input and output buffers
    double* input_buffer = new double[chunk_size];
    fftw_complex* output_buffer = new fftw_complex[chunk_size];

    // Create the FFT plan
    fftw_plan plan = fftw_plan_dft_r2c_1d(chunk_size, input_buffer, output_buffer, FFTW_ESTIMATE);

    // Open the output file
    ofstream output_file("fft_results.txt");

    // Loop over the chunks in the file
    for (int i = 0; i < num_chunks; i++) {
        // Read the chunk from the file
        file.read((char*)input_buffer, chunk_size);

        // Normalize the input data
        double max_amplitude = 0;
        for (int j = 0; j < chunk_size; j++) {
            max_amplitude = max(max_amplitude, abs(input_buffer[j]));
        }
        for (int j = 0; j < chunk_size; j++) {
            input_buffer[j] /= max_amplitude;
        }

        // Perform the FFT
        fftw_execute(plan);

        // Output the FFT results to the file
        for (int j = 0; j < chunk_size/2 + 1; j++) {
            output_file << output_buffer[j][0] << ", " << output_buffer[j][1] << endl;
        }
    }

    // Clean up
    delete[] input_buffer;
    delete[] output_buffer;
    fftw_destroy_plan(plan);
    file.close();
    output_file.close();

    return 0;
}
