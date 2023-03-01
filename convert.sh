#!/bin/bash

# Set the input and output filenames
input_file="audio.raw"
output_file="output.wav"

# Set the audio parameters
channels=1
sample_rate=44100
bitrate=16

# Use ffmpeg to convert the audio file
ffmpeg -f s16le -ar $sample_rate -ac $channels -i $input_file -b:a ${bitrate}k $output_file

# Check if the conversion was successful
if [ $? -eq 0 ]; then
    echo "Conversion successful!"
else
    echo "Conversion failed."
fi
