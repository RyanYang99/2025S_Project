#pragma once

#include <vector>

template <typename T, typename U>
class Callback {
private:
    std::vector<T> callbacks{};

public:
    void subscribe(const T callback) {
        callbacks.push_back(callback);
    }

    void unsubscribe(const T callback) noexcept {
        size_t i{};
        bool found{};

        for (; i < callbacks.size(); ++i)
            if (callbacks[i] == callback) {
                found = true;
                break;
            }

        if (!found)
            return;

        callbacks.erase(callbacks.begin() + i);
    }

    void call(const U parameter) const noexcept {
        for (const T callback : callbacks)
            callback(parameter);
    }

    void clear(void) noexcept {
        callbacks.clear();
    }
};