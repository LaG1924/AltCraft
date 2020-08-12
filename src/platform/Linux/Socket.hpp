#pragma once

#include <string>
#include <vector>
#include <stdint.h>

class Socket final {
	struct addrinfo *ai;
	int sock;

public:
	Socket(std::string &addr, uint16_t port);
	~Socket() noexcept;

	void ReadData(unsigned char *buffPtr, size_t buffLen);
	void SendData(unsigned char *buffPtr, size_t buffLen, bool more = false);

	void Connect(unsigned char *buffPtr, size_t buffLen);
};
