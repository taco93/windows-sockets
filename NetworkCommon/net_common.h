#pragma once

#include <mutex>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <ws2tcpip.h>


#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

void* get_in_addr(struct sockaddr* sa)
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

uint16_t GetPort(struct sockaddr* sa)
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

std::string PrintAddressFamily(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return "IP version 4";
	}
	else
	{
		return "IP version 6";
	}
}