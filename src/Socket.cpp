#include "Socket.hpp"

Socket::Socket(std::string address, unsigned short port) {
	sf::Socket::Status connectionStatus = socket.connect(sf::IpAddress(address), port);
	if (connectionStatus == sf::Socket::Status::Error)
		throw std::runtime_error("Can't connect to remote server");
	else if (connectionStatus != sf::Socket::Status::Done)
		throw std::runtime_error("Connection failed with unknown reason");
}

Socket::~Socket() {
	socket.disconnect();
}

void Socket::Read(unsigned char *buffPtr, size_t buffLen) {
	size_t received = 0;
	socket.receive(buffPtr, buffLen, received);
	size_t totalReceived = received;
	while (totalReceived < buffLen) {
		if (socket.receive(buffPtr + totalReceived, buffLen - totalReceived, received) != sf::Socket::Done)
			throw std::runtime_error("Raw socket data receiving is failed");
		totalReceived += received;
	}
}

void Socket::Write(unsigned char *buffPtr, size_t buffLen) {
	if (socket.send(buffPtr, buffLen) != sf::Socket::Done)
		throw std::runtime_error("Raw socket data sending is failed");
}
