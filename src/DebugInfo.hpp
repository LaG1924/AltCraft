#pragma once

#include <atomic>

struct DebugInfo {
    static std::atomic_int totalSections;
    static std::atomic_int renderSections;
    static std::atomic_int culledSections;
    static std::atomic_int readyRenderer;
    static std::atomic_int gameThreadTime;
	static std::atomic_int renderFaces;
};
