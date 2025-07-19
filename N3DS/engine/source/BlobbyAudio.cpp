#include "BlobbyAudio.h"

#define THREAD_STACK_SIZE (32 * 1024)

bool BlobbyAudio::channelUsed[24] = { false };

BlobbyAudio::BlobbyAudio() {
    LightEvent_Init(&audioEvent, RESET_ONESHOT);
}

BlobbyAudio::~BlobbyAudio() {
    Stop();
    freeResources();
}

bool BlobbyAudio::LoadPCM(const int16_t* pcmData, size_t sampleCount, int sampleRate, bool stereo) {
    Stop();  // stop current playback
    freeResources();

    int ch = allocateChannel();
    if (ch == -1) {
        printf("No free DSP channels available!\n");
        return false;
    }

    dspChannel = ch;

    size_t channels = stereo ? 2 : 1;
    size_t byteCount = sampleCount * sizeof(int16_t);

    audioBuffer = (int16_t*)linearAlloc(byteCount);
    if (!audioBuffer) {
        printf("Failed to allocate PCM audio buffer\n");
        return false;
    }

    memcpy(audioBuffer, pcmData, byteCount);

    ndspChnReset(dspChannel);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(dspChannel, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(dspChannel, sampleRate);
    ndspChnSetFormat(dspChannel, stereo ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

    memset(&waveBufs, 0, sizeof(waveBufs));
    waveBufs[0].data_vaddr = audioBuffer;
    waveBufs[0].nsamples = sampleCount;
    waveBufs[0].status = NDSP_WBUF_DONE;

    DSP_FlushDataCache(audioBuffer, byteCount);
    ndspChnWaveBufAdd(dspChannel, &waveBufs[0]);

    return true;
}

bool BlobbyAudio::LoadPCM(const std::vector<int16_t>& pcmData, int sampleRate, bool stereo) {
    return LoadPCM(pcmData.data(), pcmData.size(), sampleRate, stereo);
}

bool BlobbyAudio::LoadClip(const std::string& path) {
    Stop();
    freeResources();
    int ch = allocateChannel();
    if (ch == -1) {
        printf("No free DSP channels available!\n");
        return false;
    }
    dspChannel = ch;
    fileHandle = fopen(path.c_str(), "rb");
    if (!fileHandle) {
        printf("Failed to open file: %s\n", path.c_str());
        return false;
    }

    int error = ov_open(fileHandle, &vorbisFile, NULL, 0);
    if (error) {
        printf("Failed to decode Ogg Vorbis: %s\n", path.c_str());
        fclose(fileHandle);
        return false;
    }

    return initNDSP();
}

bool BlobbyAudio::initNDSP() {
    vorbis_info* vi = ov_info(&vorbisFile, -1);

    ndspChnReset(dspChannel);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(dspChannel, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(dspChannel, vi->rate);
    ndspChnSetFormat(dspChannel, vi->channels == 1 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_STEREO_PCM16);

    const size_t SAMPLES_PER_BUF = vi->rate * 128 / 1000; // 200ms per buffer
    const size_t CHANNELS = vi->channels;
    const size_t WAVEBUF_SIZE = SAMPLES_PER_BUF * CHANNELS * sizeof(s16);
    const size_t bufferSize = WAVEBUF_SIZE * AUDIO_BUFFER_COUNT;

    audioBuffer = (int16_t*)linearAlloc(bufferSize);
    if (!audioBuffer) {
        printf("Failed to allocate audio buffer\n");
        return false;
    }

    memset(&waveBufs, 0, sizeof(waveBufs));
    int16_t* buffer = audioBuffer;

    for (int i = 0; i < AUDIO_BUFFER_COUNT; ++i) {
        waveBufs[i].data_vaddr = buffer;
        waveBufs[i].nsamples = WAVEBUF_SIZE / sizeof(buffer[0]);
        waveBufs[i].status = NDSP_WBUF_DONE;

        buffer += WAVEBUF_SIZE / sizeof(buffer[0]);
    }

    return true;
}

int BlobbyAudio::allocateChannel() {
    for (int i = 0; i < 24; ++i) {
        if (!channelUsed[i]) {
            channelUsed[i] = true;
            return i;
        }
    }
    return -1; // none available
}

void BlobbyAudio::freeChannel(int ch) {
    if (ch >= 0 && ch < 24) {
        channelUsed[ch] = false;
    }
}

void BlobbyAudio::Play() {
    if (!fileHandle) return;

    paused = false;
    quit = false;
    ndspSetCallback(AudioCallback, this);
    SetVolume(volume);
    SetPan(stereoPan);

    int32_t priority = 0x20;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    priority = std::max(0x18, std::min(0x3F, static_cast<int>(priority - 1)));

    audioThreadId = threadCreate(AudioThreadFunc, this, THREAD_STACK_SIZE,
                                  priority, -1, false);
}

void BlobbyAudio::Stop() {
    // Signal the audio thread to exit
    if (audioThreadId) {
        quit = true;
        LightEvent_Signal(&audioEvent);  // Wake the thread if it's waiting

        threadJoin(audioThreadId, UINT64_MAX);
        threadFree(audioThreadId);
        audioThreadId = 0;
    }

    // Reset DSP channel — this will stop playback and release buffers
    currentSampleIndex = 0;
    ndspChnReset(dspChannel);
    freeChannel(dspChannel);
    dspChannel = 0;

    // Clean up Vorbis state and allocated resources
    ov_clear(&vorbisFile);
    freeResources();
}

void BlobbyAudio::Pause() {
    // Not directly supported; simulate by muting or blocking thread logic
    paused = true;
}

bool BlobbyAudio::IsPlaying() {
    for (int i = 0; i < AUDIO_BUFFER_COUNT; ++i) {
        if (waveBufs[i].status != NDSP_WBUF_DONE) {
            return true;
        }
    }
    return false;
}

void BlobbyAudio::freeResources() {
    if (audioBuffer) {
        linearFree(audioBuffer);
        audioBuffer = nullptr;
    }

    if (fileHandle) {
        fclose(fileHandle);
        fileHandle = nullptr;
    }
}

void BlobbyAudio::AudioCallback(void* arg) {
    auto* self = static_cast<BlobbyAudio*>(arg);
    if (!self->quit) {
        LightEvent_Signal(&self->audioEvent);
    }
}

void BlobbyAudio::SetPan(float pan) {
    stereoPan = std::max(-1.0f, std::min(1.0f, pan)); // clamp between -1 and 1

    float leftVol = 1.0f - std::max(0.0f, pan);   // fades right
    float rightVol = 1.0f - std::max(0.0f, -pan); // fades left

    // NDSP uses a 12-element matrix:
    // [LeftL, LeftR, LeftS1, LeftS2, RightL, RightR, RightS1, RightS2, ...]
    float mix[12] = {
        leftVol,  0.0f, 0.0f, 0.0f,  // Left output (L, R, S1, S2)
        0.0f,  rightVol, 0.0f, 0.0f, // Right output
        0.0f,  0.0f,    0.0f, 0.0f   // Unused
    };

    ndspChnSetMix(dspChannel, mix);
}

void BlobbyAudio::SetVolume(float vol) {
    volume = vol;
}

void BlobbyAudio::SetSpeed(float s) {
    speed = std::max(0.01f, s); // avoid divide-by-zero or zero playback
}

void BlobbyAudio::AudioThreadFunc(void* arg) {
    auto* self = static_cast<BlobbyAudio*>(arg);

    while (!self->quit) {
        if (self->paused) {
            svcSleepThread(1000000); // sleep 1ms
            continue;
        }
        bool didFill = false;
        for (int i = 0; i < AUDIO_BUFFER_COUNT; ++i) {
            if (self->waveBufs[i].status == NDSP_WBUF_DONE) {
                if (!self->fillBuffer(&self->waveBufs[i])) {
                    self->quit = true;
                    break;
                }
                didFill = true;
            }
        }
        if (!didFill) {
            LightEvent_Wait(&self->audioEvent);
            svcSleepThread(1000000); // sleep 1ms just in case
        }
    }
}

bool BlobbyAudio::fillBuffer(ndspWaveBuf* waveBuf) {
    if (currentSampleIndex >= 0) {
        ov_pcm_seek(&vorbisFile, currentSampleIndex); // <-- resume point
    }
    int totalBytes = 0;
    int16_t* writePtr = waveBuf->data_pcm16;

    while (totalBytes < waveBuf->nsamples * sizeof(s16)) {
        int bytesToRead = std::min((int)(sizeof(tempDecodeBuf)),
                                   (int)((waveBuf->nsamples * sizeof(s16)) - totalBytes));
        int bytesRead = ov_read(&vorbisFile, (char*)tempDecodeBuf, bytesToRead, NULL);
        if (bytesRead <= 0) {
            if (loop) {
                ov_pcm_seek(&vorbisFile, 0);
                currentSampleIndex = 0;
                continue;
            } else {
                return false;
            }
        }

        int samplesDecoded = bytesRead / sizeof(int16_t);
        currentSampleIndex += samplesDecoded;

        // Apply speed scaling (resample by skipping or interpolating)
        int targetSamples = static_cast<int>(samplesDecoded / speed);
        int16_t* resampledBuf = new int16_t[targetSamples];

        for (int i = 0; i < targetSamples; ++i) {
            float srcIndex = i * speed;
            int idx = static_cast<int>(srcIndex);

            if (idx + 1 < samplesDecoded) {
                float frac = srcIndex - idx;
                float s1 = tempDecodeBuf[idx];
                float s2 = tempDecodeBuf[idx + 1];
                resampledBuf[i] = static_cast<int16_t>((s1 * (1.0f - frac) + s2 * frac) * volume);
            } else {
                resampledBuf[i] = static_cast<int16_t>(tempDecodeBuf[idx] * volume);
            }
        }

        int chunkBytes = targetSamples * sizeof(int16_t);
        memcpy((uint8_t*)writePtr + totalBytes, resampledBuf, chunkBytes);
        totalBytes += chunkBytes;

        delete[] resampledBuf;
    }

    DSP_FlushDataCache(waveBuf->data_pcm16, totalBytes);
    waveBuf->nsamples = totalBytes / sizeof(s16);
    ndspChnWaveBufAdd(dspChannel, waveBuf);
    return true;
}
