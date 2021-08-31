#pragma once
#include "net_common.h"
#include "net_message.h"

#pragma comment (lib, "ws2_32.lib")

namespace network
{
	namespace udp
	{
		// A client is just an endpoint that is able to receive and send data using the Server/Client structure
		class Client
		{
		private:
			std::string					m_ipAddress;
			int							m_port;
			SOCKET						m_socket;
			message<MessageType>		m_message;
			struct sockaddr_storage		m_endPoint;
			socklen_t					m_endPoint_len;
			bool						m_hasConnected;

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

			m_message.header.id = MessageType::Connected;
			std::cout << "Message size should be: " << m_message.size();
			int sending = sendto(sockfd, (char*)&m_message, m_message.size(), 0, p->ai_addr, p->ai_addrlen);

			std::cout << "Bytes sent: " << sending << std::endl;

			return sockfd;
		}

		Client::Client(std::string&& ip, int&& port)
			:m_ipAddress(ip), m_port(port)
		{
			m_socket = 0;
			m_endPoint_len = sizeof(struct sockaddr_storage);
			m_endPoint = {};
			m_hasConnected = false;
			m_message = {};
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

			bool key[3] = { false, false, false };
			bool old_key[3] = { false, false, false };
			message<MessageType> buffer;

			while (1)
			{
				ZeroMemory(&buffer, sizeof(buffer));
				int8_t bytesLeft = recvfrom(m_socket, (char*)&buffer, 500, 0,
					(struct sockaddr*)&m_endPoint, &m_endPoint_len);

				if (m_hasConnected)
				{
					if (GetForegroundWindow() == GetConsoleWindow())
					{
						key[0] = GetAsyncKeyState('1') & 0x8000;
						key[1] = GetAsyncKeyState('2') & 0x8000;
						key[2] = GetAsyncKeyState('3') & 0x8000;
					}

					if (key[0] && !old_key[0])
					{
						PingServer();
					}

					for (int i = 0; i < 3; i++)
					{
						old_key[i] = key[i];
					}
				}

				if (bytesLeft > 0)
				{
					if (bytesLeft == sizeof(buffer.header))
					{
						switch (buffer.header.id)
						{
						case MessageType::Connected:
							char ipAsString[IPV6_ADDRSTRLEN];
							ZeroMemory(ipAsString, sizeof(ipAsString));
							inet_ntop(m_endPoint.ss_family, get_in_addr((struct sockaddr*)&m_endPoint), ipAsString, sizeof(ipAsString));

							std::cout << "Connected to: " << ipAsString << ":" << GetPort((struct sockaddr*)&m_endPoint) << std::endl;
							m_hasConnected = true;

							break;
						case MessageType::PingServer:
							std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
							std::chrono::system_clock::time_point timeThen;
							std::cout << buffer.size() << std::endl;
							//buffer >> timeThen;
							//std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << std::endl;

							break;
						}
					}
				}
				else
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}
					else
					{
						//std::cout << WSAGetLastError() << std::endl;
					}
				}
			}
		}
		inline void Client::PingServer()
		{
			network::message<MessageType> msg;
			msg.header.id = MessageType::PingServer;

			std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

			msg << timeNow;

			uint8_t* buffer = new uint8_t[msg.size()];

			sendto(m_socket, (char*)&msg, msg.size(), 0, (struct sockaddr*)&m_endPoint, m_endPoint_len);
		}
	}
}