# SynaesthetiQ Audio Capture

## A reduced fork of the SoundWeave program

This repo hosts a limited version of the SoundWeave library for testing in local environments. Instead of sending audio data over the internet, it is processed on the local machine.

## SynaesthetiQ AudioCapture

The audio-capture aspect of the SynaesthetiQ project utilises the ALSA (Advanced Linux Sound Architecture) to capture live audio data from hardware devices using a callback function. It uses the built in ALSA async handler callback to capture new data, and this captured data is then stored in the custom PingPongBuffer class. The ALSA callback runs in its own thread, allowing the SynaesthetiQ program to continue with other tasks while continuing to capture data in real-time. 