#pragma once

#include "../../Stream.hpp"

#include <SDL2/SDL_net.h>

class StreamSocket final : public StreamInput, public StreamOutput {
	IPaddress server;
	TCPsocket socket;

	std::vector<unsigned char> buffer;
	void ReadData(unsigned char *buffPtr, size_t buffLen) override;
	void WriteData(unsigned char *buffPtr, size_t buffLen) override;
public:
	StreamSocket(std::string &addr, Uint16 port);
	~StreamSocket() override;

	void Connect();
	void Flush();
};
