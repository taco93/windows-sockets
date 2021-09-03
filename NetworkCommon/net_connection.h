#pragma once
#include "net_common.h"
#include "net_message.h"

namespace network
{
	template <typename T>
	class connection
	{
	private:
		struct sockaddr_storage m_endpoint;
		socklen_t m_endpoint_len;

		std::string PrintSocketData(struct addrinfo* p);

	public:
		connection()
		{
			m_endpoint = {};
			m_endpoint_len = sizeof(m_endpoint);
			m_socket = INVALID_SOCKET;
		}

		virtual ~connection()
		{
			
		}

	public:
		bool ConnectToServer(const std::string& ip, const uint16_t port);
		bool IsConnected() const;
		bool Disconnect();
		bool Send(const message<T>& msg);

	protected:
		SOCKET m_socket;
	};

	template<typename T>
	inline std::string connection<T>::PrintSocketData(addrinfo* p)
	{
		std::string data = "Full socket information:\n";

		if (p->ai_family == AF_INET)
		{
			data += "Address family: AF_INET\n";
		}
		else if (p->ai_family == AF_INET6)
		{
			data += "Address family: AF_INET6\n";
		}

		if (p->ai_socktype == SOCK_STREAM)
		{
			data += "Socktype: TCP";
		}
		else if (p->ai_socktype == SOCK_DGRAM)
		{
			data += "Socktype: UDP";
		}

		data += "\n";

		return data;
	}

	template<typename T>
	inline bool connection<T>::ConnectToServer(const std::string& ip, const uint16_t port)
	{	
		// Initialize winsock
		WSADATA wsaData;

		WORD version = MAKEWORD(2, 2);

		int8_t rv = WSAStartup(version, &wsaData);

		if (rv != 0)
		{
			std::cout << "WSAStartup error code: " << WSAGetLastError() << std::endl;

			return false;
		}

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			std::cerr << "Could not find a usable version of Winsock.dll" << std::endl;

			return false;
		}

		// Get a linked network structure based on provided hints
		struct addrinfo hints, * servinfo, * p;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		rv = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &servinfo);

		if (rv != 0)
		{
			std::cerr << "Addrinfo: " << WSAGetLastError() << std::endl;
			return false;
		}

		// Loop through linked list of possible network structures
		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
			std::cout << PrintSocketData(p) << std::endl;
			m_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (m_socket == INVALID_SOCKET)
			{
				continue;
			}
			if (connect(m_socket, p->ai_addr, p->ai_addrlen) != 0)
			{
				std::cerr << "Connect error code: " << WSAGetLastError() << std::endl;
				m_socket = INVALID_SOCKET;
				closesocket(m_socket);
				continue;
			}
			break;
		}

		// Reached end of list and could not connect to any
		if (p == nullptr)
		{
			std::cerr << "Failed to connect!" << std::endl;
			m_socket = INVALID_SOCKET;

			return false;
		}

		m_endpoint = *(struct sockaddr_storage*)p->ai_addr;

		return true;
	}
	template<typename T>
	inline bool connection<T>::IsConnected() const
	{
		if (m_socket == INVALID_SOCKET)
		{
			return false;
		}
		
		return true;
	}
	template<typename T>
	inline bool connection<T>::Disconnect()
	{
		if (closesocket(m_socket) != 0)
		{
			return false;
		}

		m_socket = INVALID_SOCKET;

		return true;
	}
	template<typename T>
	inline bool connection<T>::Send(const message<T>& msg)
	{
		char buffer[BUFFER_SIZE] = "Ping Server!";

		std::cout << buffer << std::endl;

		send(m_socket, buffer, strlen(buffer), 0);

		return true;
	}
}