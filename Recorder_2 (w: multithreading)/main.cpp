// Compile with "g++ test.ccp -o test -lasound"

#include "wav.h"

int main(int argc, char *argv[])
{
    int err;
    waveRecorder recorder = waveRecorder();

    recorder.onRecord();
    scanf(" ");
    recorder.offRecord();

    return 0;
}

