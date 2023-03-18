#include "AudioCapture.h"

bool AudioCapture::quit = false;

AudioCapture::AudioCapture(const std::string &device_name, bool sdl_enabled, PingPongBuffer &buffer) : m_sdl_enabled(sdl_enabled),
                                                                                                       callback(nullptr),
                                                                                                       buffer_(buffer)

{
    std::cout << "Initialising audio hardware..." << std::endl;
    std::cout << "SDL status = " << m_sdl_enabled << std::endl;
    // int err = snd_pcm_open(&handle, "plughw:CARD=webcam,DEV=0",
    int err = snd_pcm_open(&handle, device_name.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0)
    {
        std::cerr << "Error opening PCM device: " << snd_strerror(err) << std::endl;
        throw std::runtime_error("Failed to open PCM device");
    }

    // Setup parameters
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

    unsigned int sample_rate = 44100;
    snd_pcm_hw_params_set_rate_near(handle, params, &sample_rate, nullptr);
    snd_pcm_hw_params_set_channels(handle, params, 1);
    snd_pcm_hw_params(handle, params);

    err = snd_async_add_pcm_handler(&pcm_callback, handle, &AudioCapture::MyCallback, this);
    if (err < 0)
    {
        std::cerr << "Error setting PCM async handler: " << snd_strerror(err) << std::endl;
        throw std::runtime_error("Failed to set PCM async handler");
    }

    err = snd_pcm_start(handle);
    if (err < 0)
    {
        std::cerr << "Error starting PCM device: " << snd_strerror(err) << std::endl;
        throw std::runtime_error("Failed to start PCM device");
    }

    if (m_sdl_enabled)
    {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to initialize SDL");
        }

        // Create window and renderer
        window = SDL_CreateWindow("Audio Capture", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
        if (!window)
        {
            std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to create window");
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer)
        {
            std::cerr << "Error creating renderer: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to create renderer");
        }
        waveform.reserve(800);
    }
    signal(SIGINT, signalHandler);

    // Initialize waveform data

    fftInputData.resize(4096);
    // doFFT = false;
}

AudioCapture::~AudioCapture()
{
    stopCapture();
    audioFile.close();
    snd_pcm_close(handle);
    fftInputData.clear();

    if (m_sdl_enabled)
    {

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

void AudioCapture::startCapture()
{
    quit = false;

    captureThread = std::thread([this]()
                                {
        std::cout << "Starting audio capture!" << std::endl;
        while (!quit) {

            // std::cout << "Processing" << std::endl;
            // Wait for audio data to be captured and processed by the callback function
        }
        std::cout << "STOP = " << quit << std::endl;
        std::cout << "Capture finished" << std::endl; });
}

void AudioCapture::stopCapture()
{
    if (isCapturing())
    {
        quit = true;
        captureThread.join();
    }
}

bool AudioCapture::isCapturing()
{
    return captureThread.joinable();
}

// Callback test
void AudioCapture::register_callback(DataAvailableCallback cb)
{
    callback = cb;
}

// const std::vector<int> &get_buffer() const
// {
//     return tempbuffer;
// }

void AudioCapture::MyCallback(snd_async_handler_t *pcm_callback)
{
    AudioCapture *audioCapture = reinterpret_cast<AudioCapture *>(snd_async_handler_get_callback_private(pcm_callback));
    snd_pcm_t *handle = snd_async_handler_get_pcm(pcm_callback);
    snd_pcm_sframes_t avail = snd_pcm_avail_update(handle);
    if (avail < 0)
    {
        std::cerr << "Error in snd_pcm_avail_update: " << snd_strerror(avail) << std::endl;
        return;
    }

    // std::cout << "Avail = " << avail << std::endl;

    // Create a vector to store the audio data
    std::vector<short> buffer(avail);

    // static char buffer[4096]; //1024 minimum to be safe
    snd_pcm_sframes_t frames = snd_pcm_readi(handle, buffer.data(), avail);

    // Number of samples is frames * channels
    if (frames < 0)
    {
        std::cerr << "Error in snd_pcm_readi: " << snd_strerror(frames) << std::endl;
        return;
    }

    if (buffer.size() > 2048)
    {
        std::cerr << "Buffer overflow" << std::endl;
        return;
    }


    audioCapture->buffer_.add_data(buffer);


    // Process the captured audio data in 'buffer'

    if (audioCapture->m_sdl_enabled)
    {
        // Update waveform data
        audioCapture->waveform.clear();

        for (int i = 0; i < frames; ++i)
        {
            short sample = buffer[i];
            float sampleValue = sample / static_cast<float>(SHRT_MAX);
            audioCapture->waveform.push_back(sampleValue);
        }

        // Render waveform
        SDL_SetRenderDrawColor(audioCapture->renderer, 0, 0, 0, 255);
        SDL_RenderClear(audioCapture->renderer);
        SDL_SetRenderDrawColor(audioCapture->renderer, 255, 255, 255, 255);
        for (std::vector<float>::size_type i = 0; i < audioCapture->waveform.size(); ++i)
        {
            int x = i * 800 / audioCapture->waveform.size();
            int y = (1 - audioCapture->waveform[i]) * 300 + 50;
            SDL_RenderDrawPoint(audioCapture->renderer, x, y);
            // std::cout << "x = " << x << ", y = " << y << std::endl;
            // std::cout << "test = " << audioCapture->waveform[i] << std::endl;
        }
        SDL_RenderPresent(audioCapture->renderer);
        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    buffer.clear();
}

void AudioCapture::signalHandler(int signal)
{
    std::cout << "Received signal " << signal << ". Exiting program." << std::endl;
    quit = true;
}

