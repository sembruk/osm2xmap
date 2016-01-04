#include <ctime>

#include "timer.h"

Timer::Timer() {
    startTime = std::time(nullptr);
}

double Timer::getCurTime() {
    return std::time(nullptr) - startTime;
}

