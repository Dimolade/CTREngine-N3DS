#ifndef BLOBBYAUDIO_H
#define BLOBBYAUDIO_H

#include <3ds.h>
#include <tremor/ivorbisfile.h>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <string>

#define AUDIO_BUFFER_COUNT 3

class BlobbyAudio {
private:
    OggVorbis_File vorbisFile;
    FILE* fileHandle = nullptr;
    Thread audioThreadId = 0;
    LightEvent audioEvent;
    int16_t tempDecodeBuf[4096]; // adjust as needed
    bool quit = false;

    ndspWaveBuf waveBufs[AUDIO_BUFFER_COUNT];
    int16_t* audioBuffer = nullptr;

    float volume = 1.0f;  // Default to full volume
    float speed =  1.0f;
    float stereoPan = 0.0f; // -1.0f = left, 1.0f = right

    long currentSampleIndex = 0; // sample index in the decoded stream
    bool paused = false;

    static void AudioCallback(void* arg);
    static void AudioThreadFunc(void* arg);

    int allocateChannel();
    void freeChannel(int ch);

    bool fillBuffer(ndspWaveBuf* waveBuf);
    bool initNDSP();
    void freeResources();
    u8 dspChannel = 0; // default, will assign unique value

    static bool channelUsed[24]; // NDSP supports 24 channels

public:
    BlobbyAudio();
    ~BlobbyAudio();

    bool LoadClip(const std::string& path);
    bool LoadPCM(const int16_t* pcmData, size_t sampleCount, int sampleRate, bool stereo);
    bool LoadPCM(const std::vector<int16_t>& pcmData, int sampleRate = 22050, bool stereo = false);
    void Play();
    void Pause();
    void Stop();
    bool IsPlaying();

    void SetVolume(float vol);
    void SetSpeed(float s);
    void SetPan(float p);
    float GetVolume() const { return volume; }
    float GetSpeed() const { return speed; }
    float GetPan() const { return stereoPan; }
};

#endif // BLOBBYAUDIO_H