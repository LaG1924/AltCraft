#include <iostream>
#include "Socket.hpp"

#include <thread>

Socket::Socket(std::string address, unsigned short port) {    
    if (SDLNet_Init() == -1)
        throw std::runtime_error("SDL_Net initalization failed: " + std::string(SDLNet_GetError()));

    if (SDLNet_ResolveHost(&server, address.c_str(), port) == -1)
        throw std::runtime_error("Hostname not resolved: " + std::string(SDLNet_GetError()));

    socket = SDLNet_TCP_Open(&server);
    if (!socket)
        throw std::runtime_error(std::string(SDLNet_GetError()));
}

Socket::~Socket() {
    SDLNet_TCP_Close(socket);

    SDLNet_Quit();
}

void Socket::Read(unsigned char *buffPtr, size_t buffLen) {    
    size_t totalReceived = 0;
    while (buffLen > totalReceived) {
        size_t received = SDLNet_TCP_Recv(socket, buffPtr + totalReceived, buffLen - totalReceived);
        if ( received <= 0)
            throw std::runtime_error("Data receiving failed: " + std::string(SDLNet_GetError()));
        totalReceived += received;
    }
}

void Socket::Write(unsigned char *buffPtr, size_t buffLen) {
    if (SDLNet_TCP_Send(socket, buffPtr, buffLen) < buffLen)
        throw std::runtime_error("Data sending failed: " + std::string(SDLNet_GetError()));
}
