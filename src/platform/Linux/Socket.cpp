#include "Socket.hpp"

#include <easylogging++.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
//#include <netinet/tcp.h>
#include <linux/tcp.h>

//Options from setings
unsigned int timeout=0;//msecs

// Options in advanced mode
int tfo=0;//Since Linux 3.6
int qack=1;
int nonagle=1;
int thin=1;
//End adv options

//End of options

const static int zero=0, ka_timeout=40;


Socket::Socket(std::string &address, uint16_t port) {
	int result = getaddrinfo(address.c_str(), NULL, NULL, &ai);
	if (result)
		throw std::runtime_error("Hostname not resolved: " + std::to_string(result));

	if (ai->ai_addr->sa_family == AF_INET)
		reinterpret_cast<sockaddr_in*>(ai->ai_addr)->sin_port=htons(port);
	else if (ai->ai_addr->sa_family == AF_INET6)
		reinterpret_cast<sockaddr_in*>(ai->ai_addr)->sin_port=htons(port);
	else
		throw std::runtime_error("Unknown sockaddr family");

	sock = socket(ai->ai_family, SOCK_STREAM, IPPROTO_TCP);

	setsockopt(sock, IPPROTO_TCP, TCP_FASTOPEN_CONNECT, &tfo/*TCP Fast Open enabled in network settings*/, sizeof(tfo));
	setsockopt(sock, IPPROTO_TCP, TCP_QUICKACK, &qack/*TCP Quick ACK enabled in network settings*/, sizeof(qack));
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nonagle/*Nagle disabled in network settings*/, sizeof(nonagle));
	setsockopt(sock, IPPROTO_TCP, TCP_THIN_DUPACK, &thin/*Thin stream enabled in network settings*/, sizeof(thin));
	setsockopt(sock, IPPROTO_TCP, TCP_THIN_LINEAR_TIMEOUTS, &thin/*Thin stream enabled in network settings*/, sizeof(thin));
	setsockopt(sock, IPPROTO_TCP, TCP_USER_TIMEOUT, &timeout, sizeof(timeout));
	setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &zero, sizeof(zero));
	setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &ka_timeout, sizeof(ka_timeout));
}

void Socket::Connect(unsigned char *buffPtr, size_t buffLen) {
	int result;

	result = sendto(sock, buffPtr, buffLen, MSG_FASTOPEN | MSG_NOSIGNAL, ai->ai_addr, ai->ai_addrlen);

	if (result == -1) {
		if (errno == EPIPE) {
			result = connect(sock, ai->ai_addr, ai->ai_addrlen);
			if (result != -1){
				SendData(buffPtr, buffLen);
				return;
			}
		}
		throw std::runtime_error("Connection failed: " + std::string(std::strerror(errno)));
	}
}

Socket::~Socket() noexcept {
	freeaddrinfo(ai);

	close(sock);
}

void Socket::ReadData(unsigned char *buffPtr, size_t buffLen) {
	int result;
	size_t totalReceived = 0;
	do {
		result = recv(sock, buffPtr, buffLen, MSG_WAITALL | MSG_NOSIGNAL);
		if (result == -1) {
			if(errno == EINTR)
				continue;
			else
				throw std::runtime_error("Data receiving failed: " + std::string(std::strerror(errno)));
		}
		totalReceived += result;
	} while (totalReceived < buffLen);
}

void Socket::SendData(unsigned char *buffPtr, size_t buffLen, bool more) {
	int result;
	result = send(sock, buffPtr, buffLen, MSG_DONTWAIT | MSG_NOSIGNAL | (more ? MSG_MORE : 0));
	if (result == -1)
		throw std::runtime_error("Data sending failed: " + std::string(std::strerror(errno)));
}
