#include "StreamSocket.hpp"

#include <easylogging++.h>

StreamSocket::StreamSocket(std::string &address, Uint16 port) {
	if (SDLNet_Init() == -1)
		throw std::runtime_error("SDL_Net initalization failed: " + std::string(SDLNet_GetError()));

	if (SDLNet_ResolveHost(&server, address.c_str(), port) == -1)
		throw std::runtime_error("Hostname not resolved: " + std::string(SDLNet_GetError()));
}

StreamSocket::~StreamSocket() {
	SDLNet_TCP_Close(socket);

	SDLNet_Quit();
}

void StreamSocket::ReadData(unsigned char *buffPtr, size_t buffLen) {
	size_t totalReceived = 0;
	while (buffLen > totalReceived) {
		size_t received = SDLNet_TCP_Recv(socket, buffPtr + totalReceived, buffLen - totalReceived);
		if ( received <= 0)
			throw std::runtime_error("Data receiving failed: " + std::string(SDLNet_GetError()));
		totalReceived += received;
	}
}

void StreamSocket::WriteData(unsigned char *buffPtr, size_t buffLen) {
	std::copy(buffPtr, buffPtr + buffLen, std::back_inserter(buffer));
}

void StreamSocket::Connect() {
	socket = SDLNet_TCP_Open(&server);
	if (!socket)
		LOG(WARNING) << "Connection failed: " << std::string(SDLNet_GetError());

	Flush();
}

void StreamSocket::Flush() {
	if (SDLNet_TCP_Send(socket, buffer.data(), buffer.size()) < buffer.size())
		throw std::runtime_error("Data sending failed: " + std::string(SDLNet_GetError()));
	buffer.clear();
}
