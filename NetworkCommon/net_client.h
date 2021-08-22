#pragma once
#include "net_common.h"
#include "net_message.h"

#pragma comment (lib, "ws2_32.lib")

namespace network
{
	// A client is just an endpoint that is able to receive and send data using the Server/Client structure
	class Client
	{
	private:
		std::string				m_ipAddress;
		int						m_port;
		SOCKET					m_socket;
		message<MessageType>	m_message;

	private:
		SOCKET CreateSocket();

	public:
		Client(std::string&& ip, int&& port);
		virtual ~Client();
		Client(const Client& other) = delete;
		Client& operator=(const Client& other) = delete;

	public:
		bool Init();
		void Run();
		void PingServer();
	};

	inline SOCKET Client::CreateSocket()
	{
		SOCKET sockfd = -1;
		struct addrinfo hints, * servinfo, * p;

		ZeroMemory(&hints, sizeof(hints));

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;

		int8_t rv = getaddrinfo(m_ipAddress.c_str(), std::to_string(m_port).c_str(), &hints, &servinfo);

		if (rv != 0)
		{
			std::cout << WSAGetLastError() << std::endl;
			exit(EXIT_FAILURE);
		}

		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
			sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (sockfd != -1)
			{
				break;
			}
		}

		if (p == nullptr)
		{
			std::cout << "Couldn't create socket" << std::endl;
			return -1;
		}

		u_long enable = 1;

		ioctlsocket(sockfd, FIONBIO, &enable);

		ZeroMemory(&m_message, sizeof(m_message));
		m_message.header.id = MessageType::Connected;
		sendto(sockfd, (char*)&m_message, sizeof(m_message), 0, p->ai_addr, p->ai_addrlen);

		return sockfd;
	}

	Client::Client(std::string&& ip, int&& port)
		:m_ipAddress(ip), m_port(port)
	{
		m_socket = 0;
	}

	Client::~Client()
	{
		WSACleanup();
	}

	inline bool Client::Init()
	{
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

		m_socket = CreateSocket();

		if (m_socket == -1)
		{
			return false;
		}

		return true;
	}

	inline void Client::Run()
	{
		std::cout << "[Client] Started!" << std::endl << std::endl;

		struct sockaddr_storage server_info;
		socklen_t server_info_len = sizeof(server_info);
		message<MessageType> buffer;

		while (1)
		{
			ZeroMemory(&buffer, sizeof(buffer));
			int8_t bytesLeft = recvfrom(m_socket, (char*)&buffer, sizeof(buffer), 0,
				(struct sockaddr*)&server_info, &server_info_len);

			if (bytesLeft > 0)
			{
				switch (buffer.header.id)
				{
				case MessageType::Connected:
					char ipAsString[IPV6_ADDRSTRLEN];
					ZeroMemory(ipAsString, sizeof(ipAsString));
					inet_ntop(server_info.ss_family, get_in_addr((struct sockaddr*)&server_info), ipAsString, sizeof(ipAsString));

					std::cout << "Connected to: " << ipAsString << ":" << GetPort((struct sockaddr*)&server_info) << std::endl;
					break;
				case MessageType::Ping:
					break;
				default:
					break;
				}
			}
			else
			{
			}
		}
	}
	inline void Client::PingServer()
	{
		// TODO: Add a way to ping the server for a round-trip-time printed in console.
	}
}