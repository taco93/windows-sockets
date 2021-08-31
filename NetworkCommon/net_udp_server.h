#pragma once

#include "net_common.h"
#include "net_message.h"

#pragma comment (lib, "ws2_32.lib")

// Version # 0.1 
// UDP server to handle traffic of multiple players updating their position, rotation and states

namespace network
{
	namespace udp
	{
		class Server;

		typedef void(*MessageReceivedHandler)(Server* server, struct sockaddr* sa, const socklen_t& len, message<MessageType>& msg);

		class Server
		{
		private:
			int								m_port;
			std::string						m_ipAddress;
			SOCKET							m_socket;
			MessageReceivedHandler			MessageReceived;

		private:
			SOCKET CreateSocket();

		public:
			Server(std::string&& ip, int&& port, MessageReceivedHandler handler);
			~Server();
			Server(const Server& other) = delete;
			Server& operator=(const Server& other) = delete;

			bool Init();
			void Run();
			void Send(struct sockaddr* sa, const socklen_t& len, message<MessageType>& msg);
		};

		SOCKET Server::CreateSocket()
		{
			addrinfo hints, * servinfo, * p;

			ZeroMemory(&hints, sizeof(addrinfo));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;

			int8_t rv = getaddrinfo(m_ipAddress.c_str(), std::to_string(m_port).c_str(), &hints, &servinfo);

			if (rv != 0)
			{
				std::cerr << gai_strerror(rv);
				exit(EXIT_FAILURE);
			}

			SOCKET listening = -1;

			for (p = servinfo; p != nullptr; p = p->ai_next)
			{
				listening = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

				if (listening == INVALID_SOCKET)
				{
					continue;
				}

				if (bind(listening, p->ai_addr, p->ai_addrlen) == INVALID_SOCKET)
				{
					closesocket(listening);
					continue;
				}
				break;
			}

			if (p == nullptr)
			{
				std::cerr << "Could not bind ip and port to a socket!" << std::endl;
				return -1;
			}

			freeaddrinfo(servinfo);

			u_long enable = 1;
			//ioctlsocket(listening, FIONBIO, &enable);

			return listening;
		}

		bool Server::Init()
		{
			WSADATA data;

			WORD version = MAKEWORD(2, 2);

			int8_t rv = WSAStartup(version, &data);

			if (rv != 0)
			{
				printf("Failed to initialize Winsock error code: %d\n", rv);

				return false;
			}

			m_socket = CreateSocket();

			if (m_socket == INVALID_SOCKET)
			{
				std::cerr << "Failed to create a socket error code: " << WSAGetLastError() << std::endl;
				return false;
			}

			return true;
		}

		Server::Server(std::string&& ipAddress, int&& portNumber, MessageReceivedHandler handler)
			:m_ipAddress(ipAddress), m_port(portNumber), MessageReceived(handler)
		{
			m_socket = 0;
		}

		Server::~Server()
		{
			WSACleanup();
		}

		void Server::Run()
		{
			std::cout << "[SERVER] Started!" << std::endl << std::endl;
			message<MessageType> buffer;

			while (1)
			{
				int32_t bytesReceived = 0;

				struct sockaddr_storage client_info;
				socklen_t client_info_len = sizeof(client_info);

				ZeroMemory(&buffer, sizeof(buffer));
				bytesReceived = recvfrom(m_socket, (char*)&buffer, 500, 0, (struct sockaddr*)&client_info, &client_info_len);


				if (bytesReceived > 0)
				{
					if (this->MessageReceived != nullptr)
					{
						this->MessageReceived(this, (struct sockaddr*)&client_info, client_info_len, buffer);
					}
				}
				else
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}
					else if (WSAGetLastError() == WSAETIMEDOUT)
					{
						std::cout << "Client timed out." << std::endl;
					}
					else
					{
						std::cout << WSAGetLastError() << std::endl;
					}
				}
			}
			closesocket(m_socket);
		}

		void Server::Send(struct sockaddr* sa, const socklen_t& len, message<MessageType>& msg)
		{	
			std::cout << msg << std::endl;
			switch (msg.header.id)
			{
			case MessageType::Connected:
				char ipAsString[IPV6_ADDRSTRLEN];
				ZeroMemory(ipAsString, sizeof(ipAsString));
				inet_ntop(sa->sa_family, get_in_addr(sa), ipAsString, sizeof(ipAsString));

				std::cout << "[SERVER] Client from " << ipAsString << ":" << GetPort(sa) << " " <<
					PrintAddressFamily(sa) << " " << "is accepted" << std::endl << std::endl;
				break;
			case MessageType::PingServer:
				std::cout << "[SERVER] Client requested a server ping!" << std::endl;
				break;
			}
			// This will echo back the message that was sent to the sendee
			sendto(m_socket, (char*)&msg, msg.size(), 0, sa, len);
		}
	}
}