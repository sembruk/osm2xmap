#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

class Timer {
    double startTime;
public:
    Timer();
    double getCurTime();
};

#endif // TIMER_H_INCLUDED

