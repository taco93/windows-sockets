#pragma once

#include <mutex>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <ws2tcpip.h>
#include <chrono>


#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define IPV6_ADDRSTRLEN 46

void* get_in_addr(const struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	else
	{
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
	}
}

uint16_t GetPort(const struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return ntohs(((struct sockaddr_in*)sa)->sin_port);
	}
	else
	{
		return ntohs(((struct sockaddr_in6*)sa)->sin6_port);
	}
}

std::string PrintAddressFamily(const struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return "<IP version 4>";
	}
	else
	{
		return "<IP version 6>";
	}
}

enum class MessageType
{
	Unknown,
	Connected,
	Ping
};