#include "wav.h"

//define static members
char waveRecorder::RIFF_marker[4];
char waveRecorder::filetype_header[4];
char waveRecorder::format_marker[4];
uint32_t waveRecorder::file_size = 0;
uint32_t waveRecorder::data_header_length = 16;
uint16_t waveRecorder::format_type = 1;
uint16_t waveRecorder::number_of_channels = 2;
uint32_t waveRecorder::sample_rate = 44000;
//uint32_t waveRecorder::sample_rate = 8000;
uint16_t waveRecorder::bits_per_sample = 16;
//uint16_t waveRecorder::bits_per_sample = 8;
uint32_t waveRecorder::bytes_per_second = sample_rate * number_of_channels * bits_per_sample / 8;
uint16_t waveRecorder::bytes_per_frame = number_of_channels * bits_per_sample / 8;

int waveRecorder::writeWAVHeader(int fd)
{
    write(fd, &RIFF_marker, 4);
    write(fd, &file_size, 4);
    //write(fd, &file_size, 8);
    write(fd, &filetype_header, 4);
    write(fd, &format_marker, 4);
    write(fd, &data_header_length, 4);
    write(fd, &format_type, 2);
    write(fd, &number_of_channels, 2);
    write(fd, &sample_rate, 4);
    write(fd, &bytes_per_second, 4);
    write(fd, &bytes_per_frame, 2);
    write(fd, &bits_per_sample, 2);
    write(fd, "data", 4);

    uint32_t data_size = file_size + 8 - 44;
    //uint32_t data_size = file_size;
    write(fd, &data_size, 4);

    return 0;
}

int waveRecorder::recordWAV(){
    //test initializing duation
    uint32_t duration = 0;
    //test dynamic file name
    std::string fileName, name = "test", type = ".wav";
    int part = 3;

    int err;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int sampleRate = sample_rate;
    int dir;
    snd_pcm_uframes_t frames = 32;
    char *device = (char*) "plughw:0,0";
    char *buffer;
    int filedesc;

    printf("Capture device is %s\n", device);

    /* Open PCM device for recording (capture). */
    err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err)
    {
        fprintf(stderr, "Unable to open PCM device: %s\n", snd_strerror(err));
        return err;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* ### Set the desired hardware parameters. ### */

    /* Interleaved mode */
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err)
    {
        fprintf(stderr, "Error setting interleaved mode: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Signed 16-bit little-endian format */
    if (bits_per_sample == 16) err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    else err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
    if (err)
    {
        fprintf(stderr, "Error setting format: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Two channels (stereo) */
    err = snd_pcm_hw_params_set_channels(handle, params, number_of_channels);
    if (err)
    {
        fprintf(stderr, "Error setting channels: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* 44100 bits/second sampling rate (CD quality) */
    sampleRate = sample_rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, &dir);
    if (err)
    {
        fprintf(stderr, "Error setting sampling rate (%d): %s\n", sampleRate, snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    sample_rate = sampleRate;
    /* Set period size*/
    err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    if (err)
    {
        fprintf(stderr, "Error setting period size: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0)
    {
        fprintf(stderr, "Unable to set HW parameters: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    /* Use a buffer large enough to hold one period */
    err = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    if (err)
    {
        fprintf(stderr, "Error retrieving period size: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    size = frames * bits_per_sample / 8 * number_of_channels; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);
    if (!buffer)
    {
        fprintf(stdout, "Buffer error.\n");
        snd_pcm_close(handle);
        return -1;
    }

    err = snd_pcm_hw_params_get_period_time(params, &sampleRate, &dir);
    if (err)
    {
        fprintf(stderr, "Error retrieving period time: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        free(buffer);
        return err;
    }

    fileName = name + std::to_string(part) + type;

    uint32_t pcm_data_size = sample_rate * bytes_per_frame * duration / 1000;
    file_size = pcm_data_size + 44 - 8;
    //file_size = pcm_data_size;

    filedesc = open(fileName.c_str(), O_WRONLY | O_CREAT, 0644);
    err = writeWAVHeader(filedesc);
    if (err)
    {
        fprintf(stderr, "Error writing .wav header.");
        snd_pcm_close(handle);
        free(buffer);
        close(filedesc);
        return err;
    }
    
    fprintf(stdout, "Channels: %d\n", number_of_channels);
    for(duration = 0; duration < 10; duration ++){
        printf("___duration: %d\n", duration);
        for(int i = ( (1000*1000) / (sample_rate / frames)); i > 0; i--)
        {
            err = snd_pcm_readi(handle, buffer, frames);
            if (err == -EPIPE) fprintf(stderr, "Overrun occurred: %d\n", err);
            if (err < 0) err = snd_pcm_recover(handle, err, 0);

            if (err < 0)
            {
                fprintf(stderr, "Error occured while recording: %s\n", snd_strerror(err));
                snd_pcm_close(handle);
                free(buffer);
                close(filedesc);
                return err;
            }
            write(filedesc, buffer, size);
        }
    }

    close(filedesc);

    duration *= 1000;
    //Rewrite the header
    pcm_data_size = sample_rate * bytes_per_frame * duration / 1000;
    file_size = pcm_data_size + 44 - 8;
    //file_size = pcm_data_size;    

    filedesc = open(fileName.c_str(), O_WRONLY | O_CREAT, 0644);
    err = writeWAVHeader(filedesc);
    close(filedesc);

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);

    printf("Finished writing to %s\n", fileName.c_str());
    return 0;
}

waveRecorder::waveRecorder(){
    memcpy(&RIFF_marker, "RIFF", 4);
    memcpy(&filetype_header, "WAVE", 4);
    memcpy(&format_marker, "fmt ", 4);
}

