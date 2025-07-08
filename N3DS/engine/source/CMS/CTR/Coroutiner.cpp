#include "Coroutiner.h"

void Coroutiner::CoroutineFunction(int frameCount, Callback func) {
    running.push_back({frameCount, 0, func});
}

void Coroutiner::Update() {
    for (auto it = running.begin(); it != running.end(); ) {
        it->callback(it->currentFrame);
        it->currentFrame++;

        if (it->currentFrame >= it->totalFrames) {
            it = running.erase(it); // done
        } else {
            ++it;
        }
    }
}
