#include "BlobbyAudio.h"
#include "CMS/CTR/Debug.hpp"

#define THREAD_STACK_SIZE (32 * 1024)

bool BlobbyAudio::channelUsed[24] = { false };

BlobbyAudio::BlobbyAudio() {
    //LightEvent_Init(&audioEvent, RESET_ONESHOT);
}

BlobbyAudio::~BlobbyAudio() {
    Stop();
    freeResources();
}

bool BlobbyAudio::LoadPCMClip(const std::string& path, int sampleRate, bool stereo)
{
    Stop();
    freeResources();

    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
        printf("Failed to open raw PCM file: %s\n", path.c_str());
        return false;
    }

    fseek(f, 0, SEEK_END);
    size_t fileSize = ftell(f);
    rewind(f);

    int sampleCount = fileSize / sizeof(int16_t);
    audioBuffer = (int16_t*)linearAlloc(fileSize);
    if (!audioBuffer) {
        fclose(f);
        printf("Failed to allocate audio buffer\n");
        return false;
    }

    fread(audioBuffer, 1, fileSize, f);
    fclose(f);

    int ch = allocateChannel();
    if (ch == -1) {
        printf("No DSP channels available\n");
        return false;
    }

    dspChannel = ch;

    ndspChnReset(dspChannel);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(dspChannel, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(dspChannel, sampleRate);
    ndspChnSetFormat(dspChannel, stereo ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

    memset(&waveBufs, 0, sizeof(waveBufs));
    waveBufs[0].data_vaddr = audioBuffer;
    waveBufs[0].nsamples = sampleCount;
    waveBufs[0].status = NDSP_WBUF_DONE;
    waveBufs[0].looping = loop;

    DSP_FlushDataCache(audioBuffer, fileSize);
    ndspChnWaveBufAdd(dspChannel, &waveBufs[0]);

    return true;
}

bool BlobbyAudio::LoadClip(const std::string& path) {
    return LoadPCMClip(path, 48000, false);
    /*Stop();
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

    bool fi = initNDSP();
	
	for (int i = 0; i < AUDIO_BUFFER_COUNT; ++i) {
		fillBuffer(&waveBufs[i]); // Pre-fill
		ndspChnWaveBufAdd(dspChannel, &waveBufs[i]);
	}

    return fi;*/
}

/*bool BlobbyAudio::initNDSP() {
    vorbis_info* vi = ov_info(&vorbisFile, -1);

    ndspChnReset(dspChannel);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(dspChannel, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(dspChannel, vi->rate);
    ndspChnSetFormat(dspChannel, vi->channels == 1 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_STEREO_PCM16);

    const size_t SAMPLES_PER_BUF = vi->rate * 120 / 1000; // ~200ms
    const size_t CHANNELS = vi->channels;
    const size_t WAVEBUF_SIZE = SAMPLES_PER_BUF * CHANNELS * sizeof(int16_t);
    const size_t bufferSize = WAVEBUF_SIZE * AUDIO_BUFFER_COUNT;

    audioBuffer = (int16_t*)linearAlloc(bufferSize);
    if (!audioBuffer) {
        printf("Failed to allocate audio buffer.\n");
        return false;
    }

    memset(&waveBufs, 0, sizeof(waveBufs));
    int16_t* buffer = audioBuffer;

    for (int i = 0; i < AUDIO_BUFFER_COUNT; ++i) {
        waveBufs[i].data_vaddr = buffer;
        waveBufs[i].nsamples = WAVEBUF_SIZE / sizeof(int16_t);
        waveBufs[i].status = NDSP_WBUF_DONE;
        buffer += WAVEBUF_SIZE / sizeof(int16_t);
    }

    size_t maxNeededSamples = (WAVEBUF_SIZE / sizeof(int16_t)) * 2;
    resampleBuf = (int16_t*)linearAlloc(maxNeededSamples * sizeof(int16_t));
    if (!resampleBuf) {
        printf("Failed to allocate resample buffer.\n");
        linearFree(audioBuffer);
        audioBuffer = nullptr;
        return false;
    }

    return true;
}*/

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
    if (!audioBuffer) return;

    paused = false;
    quit = false;
    //ndspSetCallback(AudioCallback, this);
    SetVolume(volume);
    SetPan(stereoPan);

    /*return;

    int32_t priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    // ... then subtract 1, as lower number => higher actual priority ...
    priority -= 1;
    // ... finally, clamp it between 0x18 and 0x3F to guarantee that it's valid.
    priority = priority < 0x18 ? 0x18 : priority;
    priority = priority > 0x3F ? 0x3F : priority;

    audioThreadId = threadCreate(AudioThreadFunc, this, THREAD_STACK_SIZE,
                                  priority, -1, false);*/
}

void BlobbyAudio::Stop() {
    // Stop the audio thread safely
	/*Log::Append("Stop Called.\n");
    Log::Save();
    if (audioThreadId) {
        quit = true;
        LightEvent_Signal(&audioEvent);  // Wake the thread if it's waiting

        threadJoin(audioThreadId, UINT64_MAX);
        threadFree(audioThreadId);
        audioThreadId = 0;
    }*/

    // Reset current position and set to paused
    currentSampleIndex = 0;
    paused = true;

    // Stop DSP playback (don't release buffers or destroy DSP channel)
    ndspChnReset(dspChannel);

    // NOTE: Do NOT call freeChannel(), ov_clear(), or freeResources() here
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
	/*Log::Append("Freeing Resources.\n");
    Log::Save();*/
    if (audioBuffer) {
        linearFree(audioBuffer);
        audioBuffer = nullptr;
    }

    /*if (fileHandle) {
        fclose(fileHandle);
        fileHandle = nullptr;
    }
	
	if (resampleBuf) {
		linearFree(resampleBuf);
		resampleBuf = nullptr;
	}*/
}

/*void BlobbyAudio::AudioCallback(void* arg) {
    auto* self = static_cast<BlobbyAudio*>(arg);
    if (!self->quit) {
        LightEvent_Signal(&self->audioEvent);
    }
}*/

void BlobbyAudio::Update()
{
    if (loop && waveBufs[0].status == NDSP_WBUF_DONE)
    {
        waveBufs[0].status = NDSP_WBUF_DONE;
        ndspChnWaveBufAdd(dspChannel, &waveBufs[0]);
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
    SetPan(stereoPan);
}

void BlobbyAudio::SetSpeed(float s) {
    speed = std::max(0.01f, s); // avoid divide-by-zero or zero playback
    ndspChnSetRate(dspChannel, 48000.0f * speed);
}

/*void BlobbyAudio::AudioThreadFunc(void* arg) {
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
        }
    }
}*/

/*bool BlobbyAudio::fillBuffer(ndspWaveBuf* waveBuf) {
    // Remove redundant seek — only seek on loop or resume
    int totalBytes = 0;
    int16_t* writePtr = static_cast<int16_t*>(const_cast<void*>(waveBuf->data_vaddr));
    const size_t maxBytes = waveBuf->nsamples * sizeof(int16_t);

    while (totalBytes < maxBytes) {
        int bytesToRead = std::min((int)(sizeof(tempDecodeBuf)), (int)(maxBytes - totalBytes));
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

        int targetSamples = static_cast<int>(samplesDecoded / speed);

        // Ensure we don’t exceed buffer size
        if (targetSamples > MAX_RESAMPLE_SAMPLES) {
            targetSamples = MAX_RESAMPLE_SAMPLES;
        }

        for (int i = 0; i < targetSamples; ++i) {
            float srcIndex = i * speed;
            int idx = static_cast<int>(srcIndex);

            if (idx + 1 < samplesDecoded) {
                float frac = srcIndex - idx;
                float s1 = tempDecodeBuf[idx];
                float s2 = tempDecodeBuf[idx + 1];
                resampleBuf[i] = static_cast<int16_t>((s1 * (1.0f - frac) + s2 * frac) * volume);
            } else if (idx < samplesDecoded) {
                resampleBuf[i] = static_cast<int16_t>(tempDecodeBuf[idx] * volume);
            } else {
                break; // Prevent over-read
            }
        }

        int chunkBytes = targetSamples * sizeof(int16_t);

        // Avoid writing past the wave buffer
        if (totalBytes + chunkBytes > maxBytes) {
            chunkBytes = maxBytes - totalBytes;
        }

        memcpy((uint8_t*)writePtr + totalBytes, resampleBuf, chunkBytes);
        totalBytes += chunkBytes;
    }

    DSP_FlushDataCache(writePtr, totalBytes);
    waveBuf->nsamples = totalBytes / sizeof(int16_t);
    ndspChnWaveBufAdd(dspChannel, waveBuf);
    return true;
}*/
