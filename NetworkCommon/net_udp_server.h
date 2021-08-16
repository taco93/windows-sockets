#pragma once

#include "net_common.h"
#pragma comment (lib, "ws2_32.lib")

#define BUFFER_SIZE 4096
// Version # 0.1 
// UDP server to handle traffic of multiple players updating their position, rotation and states

namespace network
{
	namespace udp
	{
		struct ClientMetaData
		{
			sockaddr_storage addr;
			socklen_t addrLen = sizeof(sockaddr_storage);
			char ipAsString[IPV6_ADDRSTRLEN];
			UINT id;
		};

		class Server;

		typedef void(*MessageReceivedHandler)(Server* server, struct sockaddr* sa, const socklen_t& len, const std::string& msg);

		class Server
		{
		private:
			int								m_port;
			std::string						m_ipAddress;
			SOCKET							m_socket;
			std::vector<ClientMetaData>		m_clients;
			MessageReceivedHandler			MessageReceived;

		private:
			SOCKET CreateSocket();

		public:
			Server(std::string&& ip, int&& port, MessageReceivedHandler handler);
			~Server();

			bool Init();
			void Run();
			void Send(struct sockaddr* sa, const socklen_t& len, const std::string& msg);
		};

		SOCKET Server::CreateSocket()
		{
			addrinfo hints, * servinfo, * p;

			ZeroMemory(&hints, sizeof(addrinfo));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;

			int rv = getaddrinfo(m_ipAddress.c_str(), std::to_string(m_port).c_str(), &hints, &servinfo);

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
			ioctlsocket(listening, FIONBIO, &enable);

			return listening;
		}

		bool Server::Init()
		{
			WSADATA data;

			WORD version = MAKEWORD(2, 2);

			int rv = WSAStartup(version, &data);

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
			std::cout << "[SERVER] Started!" << std::endl;

			char buffer[BUFFER_SIZE];

			while (1)
			{
				int bytesReceived = 0;

				char ipAsString[IPV6_ADDRSTRLEN];
				ZeroMemory(ipAsString, sizeof(ipAsString));

				struct sockaddr_storage client_info;
				socklen_t client_info_len = sizeof(client_info);

				ZeroMemory(buffer, sizeof(buffer));
				bytesReceived = recvfrom(m_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_info, &client_info_len);

				if (bytesReceived > 0)
				{
					inet_ntop(client_info.ss_family, get_in_addr((struct sockaddr*)&client_info), ipAsString, sizeof(ipAsString));
					std::cout << "[SERVER] Incoming message from: " << ipAsString << ":" << GetPort((struct sockaddr*)&client_info) << " " <<
						PrintAddressFamily((struct sockaddr*)&client_info) << std::endl;

					std::cout << "Message: " << buffer << std::endl;
					if (this->MessageReceived != NULL)
					{
						this->MessageReceived(this, (struct sockaddr*)&client_info, client_info_len, std::string(buffer, 0, bytesReceived));
					}
				}
			}
			closesocket(m_socket);
		}

		void Server::Send(struct sockaddr* sa, const socklen_t& len, const std::string& msg)
		{
			sendto(m_socket, msg.c_str(), msg.size() + 1, 0, sa, len);
		}
	}
}