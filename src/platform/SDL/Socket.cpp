#include "Socket.hpp"

#include <easylogging++.h>

Socket::Socket(std::string &address, Uint16 port) {
	if (SDLNet_Init() == -1)
		throw std::runtime_error("SDL_Net initalization failed: " + std::string(SDLNet_GetError()));

	if (SDLNet_ResolveHost(&server, address.c_str(), port) == -1)
		throw std::runtime_error("Hostname not resolved: " + std::string(SDLNet_GetError()));
}

Socket::~Socket() noexcept {
	SDLNet_TCP_Close(socket);

	SDLNet_Quit();
}

void Socket::ReadData(unsigned char *buffPtr, size_t buffLen) {
	size_t totalReceived = 0;
	while (buffLen > totalReceived) {
		size_t received = SDLNet_TCP_Recv(socket, buffPtr + totalReceived, buffLen - totalReceived);
		if ( received <= 0)
			throw std::runtime_error("Data receiving failed: " + std::string(SDLNet_GetError()));
		totalReceived += received;
	}
}

void Socket::SendData(unsigned char *buffPtr, size_t buffLen, bool more) {
	if (more || !buffer.empty()) {
		std::copy(buffPtr, buffPtr + buffLen, std::back_inserter(buffer));
		if (!more)
			Flush();
	} else {
		if (SDLNet_TCP_Send(socket, buffPtr, buffLen) < buffLen)
				throw std::runtime_error("Data sending failed: " + std::string(SDLNet_GetError()));
	}
}

void Socket::Connect(unsigned char *buffPtr, size_t buffLen) {
	socket = SDLNet_TCP_Open(&server);
	if (!socket)
		LOG(WARNING) << "Connection failed: " << std::string(SDLNet_GetError());
	if (buffLen)
		SendData(buffPtr, buffLen);
}

void Socket::Flush() {
	if (SDLNet_TCP_Send(socket, buffer.data(), buffer.size()) < buffer.size())
		throw std::runtime_error("Data sending failed: " + std::string(SDLNet_GetError()));
	buffer.clear();
}
