#pragma once

#include "../../Stream.hpp"

class StreamSocket final : public StreamInput, public StreamOutput {
//	IPaddress server;
	struct addrinfo *ai;
	int sock;

	std::vector<unsigned char> buffer;
	void ReadData(unsigned char *buffPtr, size_t buffLen) override;
	void WriteData(unsigned char *buffPtr, size_t buffLen) override;
public:
	StreamSocket(std::string &addr, uint16_t port);
	~StreamSocket() override;

	void Connect();
	void Flush();
};
