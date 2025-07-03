#ifndef EVENTS_H
#define EVENTS_H

#include <functional>
#include <vector>

class Event
{
public:
    // Define a type alias for the callback
    using Callback = std::function<void()>;

    // Subscribe a listener
    void Subscribe(Callback callback)
    {
        listeners.push_back(callback);
    }

    // Trigger the event
    void Trigger()
    {
        for (auto& callback : listeners)
        {
            if (callback) callback();
        }
    }

private:
    std::vector<Callback> listeners;
};

#endif