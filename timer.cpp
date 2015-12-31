#include <ctime>

#include "timer.h"

Timer::Timer() {
    startTime = std::time(NULL);
}

double Timer::getCurTime() {
    return std::time(NULL) - startTime;
}

