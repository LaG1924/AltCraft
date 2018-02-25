#pragma once

#include <algorithm>
#include <string>
#include <chrono>
#include <vector>

#include <GL/glew.h>

using Uuid = std::vector<unsigned char>;

template<class T>
void endswap(T *objp) {
    unsigned char *memp = reinterpret_cast<unsigned char *>(objp);
    std::reverse(memp, memp + sizeof(T));
}

template<class T>
void endswap(T &obj) {
    unsigned char *raw = reinterpret_cast<unsigned char *>(&obj);
    std::reverse(raw, raw + sizeof(T));
}

template<typename T>
T _min(T a, T b) {
    return (a > b) ? b : a;
}

template<typename T, typename... Args>
T _min(T a, T b, Args... args) {
    return _min(a > b ? b : a, args...);
}

template<typename T>
T _max(T a, T b) {
    return (a > b) ? a : b;
}

template<typename T, typename... Args>
T _max(T a, T b, Args... args) {
    return _max(a > b ? a : b, args...);
}

inline void endswap(unsigned char *arr, size_t arrLen) {
    std::reverse(arr, arr + arrLen);
}

GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)


class LoopExecutionTimeController {
    using clock = std::chrono::steady_clock ;
    using timePoint = std::chrono::time_point<clock>;
    using duration = std::chrono::duration<double,std::milli>;
    timePoint previousUpdate;
    timePoint previousPreviousUpdate;
    duration delayLength;
    unsigned long long iterations=0;
public:
    LoopExecutionTimeController(duration delayLength);

    ~LoopExecutionTimeController();

    void SetDelayLength(duration length);

    unsigned long long GetIterations();

    void Update();

    double GetDeltaMs();

    duration GetDelta();

    double GetDeltaS();

    double GetRealDeltaS();

    double RemainTimeMs();
};