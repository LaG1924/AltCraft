#include "DebugInfo.hpp"

std::atomic_int DebugInfo::totalChunks(0);
std::atomic_int DebugInfo::renderSections(0);
std::atomic_int DebugInfo::readyRenderer(0);
std::atomic_int DebugInfo::gameThreadTime(0);
std::atomic_int DebugInfo::renderFaces(0);
