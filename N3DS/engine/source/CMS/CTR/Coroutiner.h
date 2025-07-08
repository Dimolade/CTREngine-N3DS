#pragma once
#include <vector>
#include <functional>

class Coroutiner {
public:
    using Callback = std::function<void(int)>;

    static void CoroutineFunction(int frameCount, Callback func);
    static void Update();

private:
    struct TimedCall {
        int totalFrames;
        int currentFrame;
        Callback callback;
    };

    static inline std::vector<TimedCall> running;
};
