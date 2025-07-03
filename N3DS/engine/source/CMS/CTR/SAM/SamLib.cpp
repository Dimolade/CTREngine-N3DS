#include "SamLib.h"

extern "C" {
    #include "reciter.h"
    #include "sam.h"
}

#include <cstring>
#include <cctype>

SAM::SAM() {
    SetSpeed(72);
    SetPitch(64);
    SetMouth(128);
    SetThroat(128);
    // EnableSingmode(); // Optional
}

std::vector<int16_t> SAM::Say(const std::string& text) {
    return SayPCM16(text);
}

std::vector<int8_t> SAM::SayPCM8(const std::string& text) {
    std::vector<int8_t> pcm;

    char input[256] = {0};
    strncpy(input, text.c_str(), sizeof(input) - 2);
    for (int i = 0; input[i]; ++i) input[i] = std::toupper(static_cast<unsigned char>(input[i]));

    strcat(input, "[");

    if (!TextToPhonemes(reinterpret_cast<unsigned char*>(input)))
        return pcm;

    SetInput(reinterpret_cast<unsigned char*>(input));

    if (!SAMMain())
        return pcm;

    char* buffer = GetBuffer();
    int len = GetBufferLength() / 50;

    pcm.reserve(len);
    for (int i = 0; i < len; ++i) {
        pcm.push_back(buffer[i] - 128);
    }

    return pcm;
}

std::vector<int16_t> SAM::SayPCM16(const std::string& text) {
    std::vector<int16_t> pcm;

    char input[256] = {0};
    strncpy(input, text.c_str(), sizeof(input) - 2);
    for (int i = 0; input[i]; ++i)
        input[i] = std::toupper(static_cast<unsigned char>(input[i]));

    strcat(input, "[");

    if (!TextToPhonemes(reinterpret_cast<unsigned char*>(input)))
        return pcm;

    SetInput(reinterpret_cast<unsigned char*>(input));

    if (!SAMMain())
        return pcm;

    char* buffer = GetBuffer();
    int len = GetBufferLength() / 50;

    pcm.reserve(len);
    for (int i = 0; i < len; ++i) {
        // Convert 8-bit unsigned to 16-bit signed (scale to full 16-bit range)
        int16_t sample = static_cast<int16_t>((buffer[i] - 128) << 8);
        pcm.push_back(sample);
    }

    return pcm;
}


void SAM::SetSpeed(int val) { SetSpeed(val); }
void SAM::SetPitch(int val) { SetPitch(val); }
void SAM::SetMouth(int val) { SetMouth(val); }
void SAM::SetThroat(int val) { SetThroat(val); }

// Optional SaveWav method can be restored if needed

