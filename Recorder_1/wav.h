//wav.h

#ifndef WAV_H
#define WAV_H

// Compile with "g++ test.ccp -o test -lasound"

// Use the newer ALSA API
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string>

class waveRecorder {
public:
    static char RIFF_marker[4];
    static uint32_t file_size;
    static char filetype_header[4];
    static char format_marker[4];
    static uint32_t data_header_length;
    static uint16_t format_type;
    static uint16_t number_of_channels;
    static uint32_t sample_rate;
    static uint32_t bytes_per_second;
    static uint16_t bytes_per_frame;
    static uint16_t bits_per_sample;
    
    int writeWAVHeader(int fd);
    int recordWAV();

    //constructor
    waveRecorder();
};

#endif

