#include "Utility.hpp"

#include <thread>

#include <optick.h>
#include <easylogging++.h>

LoopExecutionTimeController::LoopExecutionTimeController(duration delayLength)
        : delayLength(delayLength) {
    previousUpdate = clock::now();
}

void LoopExecutionTimeController::SetDelayLength(duration length) {
    delayLength = length;
}

unsigned long long LoopExecutionTimeController::GetIterations() {
    return iterations;
}

void LoopExecutionTimeController::Update() {
    iterations++;
    auto timeToSleep = delayLength - GetDelta();    
    if (timeToSleep.count() > 0)
        std::this_thread::sleep_for(timeToSleep);
    previousPreviousUpdate = previousUpdate;
    previousUpdate = clock::now();    
}

double LoopExecutionTimeController::GetDeltaMs() {
    std::chrono::duration<double, std::milli> delta = GetDelta();    
    return delta.count();
}

LoopExecutionTimeController::duration LoopExecutionTimeController::GetDelta() {
    auto now = clock::now();
    return duration(now-previousUpdate);
}

double LoopExecutionTimeController::GetDeltaS() {
    std::chrono::duration<double, std::ratio<1,1>> delta = GetDelta();
    return delta.count();
}

double LoopExecutionTimeController::GetRealDeltaS() {
    return std::chrono::duration<double,std::ratio<1,1>>(previousUpdate - previousPreviousUpdate).count();
}

double LoopExecutionTimeController::RemainTimeMs() {
    auto remain = delayLength - GetDelta();
    return remain.count();
}
