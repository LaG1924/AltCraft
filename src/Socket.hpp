#pragma once

#include <string>

#include <SFML/Network.hpp>

#include <SDL_net.h>

/**
 * Platform independent class for working with platform dependent hardware socket
 * @brief Wrapper around raw sockets
 * @warning Connection state is based on lifetime of Socket object instance, ie connected at ctor and disconnect at dtor
 */
class Socket {
    IPaddress server;
    TCPsocket socket;
public:
	/**
	 * Constructs Socket class instance from IP's string and Port number and connects to remote server
	 * @param[in] address IP address of remote server. String should be ANSI and contains 4 one-byte values separated by dots
	 * @param[in] port target port of remote server to connect
	 * @throw std::runtime_error if connection is failed
	 */
	Socket(std::string address, unsigned short port);

	/**
	 * Destruct Socket instance and disconnect from server
	 * @warning There is no way to force disconnect, except use delete for manually allocated objects and scope of visibility for auto variables
	 */
	~Socket();

	/**
	 * Reads data from socket and write to buffer
	 * @warning This is blocking function, and execution flow will not be returned until all required data is sended
	 * @warning Reported buffer length must be <= actual size of buffer, or memory corruption will be caused
	 * @param[out] buffPtr Pointer to buffer, where data must be placed
	 * @param[in] buffLen Length of data, that must be readed from server and writed to buffer
	 */
	void Read(unsigned char *buffPtr, size_t buffLen);

	/**
	 * Writes data from buffer to socket
	 * @warning This is blocking function, and execution flow will not be returned until all required data is received
	 * @param[in] buffPtr Pointer to buffer that contain data to send
	 * @param[in] buffLen Length of buffer
	 */
	void Write(unsigned char *buffPtr, size_t buffLen);
};