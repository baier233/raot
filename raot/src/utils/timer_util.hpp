#pragma once
#include <chrono>
namespace utils {
    class TimerUtil {

    private:
        std::chrono::steady_clock::time_point lastMS = std::chrono::steady_clock::now();

    public:
       inline void reset() {
            lastMS = std::chrono::steady_clock::now();
        }

       inline bool has_time_elapsed(std::chrono::milliseconds time, bool reset) {
            std::chrono::steady_clock::time_point timerNow = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(timerNow - lastMS) > time) {
                if (reset) this->reset();
                return true;
            }
            return false;
        }

        template <typename T>
        inline bool has_time_elapsed(T time, bool reset) {
            std::chrono::milliseconds duration(time);
            return has_time_elapsed(duration, reset);
        }
    };

}
