#pragma once

#ifdef __linux__
#include "platform/Linux/StreamSocket.hpp"
#else
#include "platform/SDL/StreamSocket.hpp"
#endif
