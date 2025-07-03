#ifndef SAM_LIB_H
#define SAM_LIB_H

#include <string>
#include <vector>
#include <cstdint>

class SAM {
public:
    SAM();

    std::vector<int8_t> SayPCM8(const std::string& text);
    std::vector<int16_t> SayPCM16(const std::string& text);
    std::vector<int16_t> Say(const std::string& text);

    void SetSpeed(int val);
    void SetPitch(int val);
    void SetMouth(int val);
    void SetThroat(int val);

    // Optional:
    // static void SaveWav(const std::string& filename, const std::vector<int8_t>& pcm);
};

#endif // SAM_LIB_H
