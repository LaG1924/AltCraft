#pragma once

#ifndef __linux__
#include "Linux/StreamSocket.hpp"
#else
#include "SDL/StreamSocket.hpp"
#endif
