#pragma once

#ifdef __linux__
#include "Linux/Socket.hpp"
#else
#include "SDL/Socket.hpp"
#endif
