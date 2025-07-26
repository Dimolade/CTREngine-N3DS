#ifndef BLOBBYAUDIO_H
#define BLOBBYAUDIO_H

#include <3ds.h>
//#include <tremor/ivorbisfile.h>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <string>

#define AUDIO_BUFFER_COUNT 3

class BlobbyAudio {
private:
    //OggVorbis_File vorbisFile;
    //FILE* fileHandle = nullptr;
    Thread audioThreadId = 0;
    LightEvent audioEvent;
    //int16_t tempDecodeBuf[4096]; // adjust as needed
    //int16_t* resampleBuf = nullptr;
	static const size_t MAX_RESAMPLE_SAMPLES = 65536;
    bool quit = false;

    ndspWaveBuf waveBufs[AUDIO_BUFFER_COUNT];
    int16_t* audioBuffer = nullptr;

    float volume = 1.0f;  // Default to full volume
    float speed =  1.0f;
    float stereoPan = 0.0f; // -1.0f = left, 1.0f = right
    bool loop = false;

    long currentSampleIndex = 0; // sample index in the decoded stream
    bool paused = false;

    //static void AudioCallback(void* arg);
    //static void AudioThreadFunc(void* arg);

    int allocateChannel();
    void freeChannel(int ch);

    //bool fillBuffer(ndspWaveBuf* waveBuf);
    //bool initNDSP();
    void freeResources();
    u8 dspChannel = 0; // default, will assign unique value

    static bool channelUsed[24]; // NDSP supports 24 channels

public:
    BlobbyAudio();
    ~BlobbyAudio();

    bool LoadClip(const std::string& path);
    bool LoadPCMClip(const std::string& path, int sampleRate = 48000, bool stereo = false);
    void Play();
    void Pause();
    void Stop();
    bool IsPlaying();
    void Update();

    void SetVolume(float vol);
    void SetSpeed(float s);
    void SetPan(float p);
    void SetLooping(bool enabled) { loop = enabled; }
    float GetVolume() const { return volume; }
    float GetSpeed() const { return speed; }
    float GetPan() const { return stereoPan; }
    bool IsLooping() const { return loop; }
};

#endif // BLOBBYAUDIO_H