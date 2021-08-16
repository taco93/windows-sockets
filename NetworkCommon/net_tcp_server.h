#pragma once
#include "net_common.h"

#pragma comment (lib, "ws2_32.lib")

#define BUFFER_SIZE 4096

namespace network
{
	namespace tcp
	{
		struct ClientMetaData
		{
			sockaddr_storage addr;
			socklen_t addrLen = sizeof(sockaddr_storage);
			SOCKET socket;
			char ipAsString[IPV6_ADDRSTRLEN];
			UINT id;
		};

		class Server;

		typedef void(*MessageReceivedHandler)(Server* server, const int& socketId, const std::string& msg);

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
			SOCKET WaitForConnection(const SOCKET& listening, ClientMetaData* output);

		public:
			Server(std::string&& ip, int&& port, MessageReceivedHandler handler);
			~Server();

			bool Init();
			void Run();
			void Send(const int& clientSocket, const std::string& msg);
		};

		SOCKET Server::CreateSocket()
		{
			addrinfo hints, * servinfo, * p;

			ZeroMemory(&hints, sizeof(addrinfo));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;

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
				std::cerr << "Could not bind ip and port to a socket, aborting!" << std::endl;
				return -1;
			}

			freeaddrinfo(servinfo);

			u_long enable = 1;
			ioctlsocket(listening, FIONBIO, &enable);

			if (listen(listening, SOMAXCONN) == INVALID_SOCKET)
			{
				std::cerr << WSAGetLastError() << std::endl;
				return -1;
			}

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

		SOCKET Server::WaitForConnection(const SOCKET& listening, ClientMetaData* output)
		{
			if (output == nullptr)
			{
				return accept(listening, nullptr, nullptr);
			}

			output->socket = accept(listening, (struct sockaddr*)&output->addr, &output->addrLen);

			return output->socket;
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

			fd_set master = {};
			FD_SET(m_socket, &master);
			char buffer[BUFFER_SIZE];

			while (1)
			{
				fd_set copy = master;

				int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

				for (int i = 0; i < socketCount; i++)
				{
					SOCKET currentSocket = copy.fd_array[i];

					if (FD_ISSET(currentSocket, &copy))
					{
						if (currentSocket == m_socket)
						{
							ClientMetaData client = {};
							client.socket = WaitForConnection(m_socket, &client);

							if (client.socket == INVALID_SOCKET)
							{
								return;
							}
							inet_ntop(client.addr.ss_family, get_in_addr((struct sockaddr*)&client.addr), &client.ipAsString[0], IPV6_ADDRSTRLEN);
							std::cout << "[SERVER] Client connected from: " << client.ipAsString << ":" << GetPort((struct sockaddr*)&client.addr) << " " <<
								PrintAddressFamily((struct sockaddr*)&client.addr) << std::endl;
							m_clients.push_back(client);

							FD_SET(client.socket, &master);
						}
						else
						{
							int bytesReceived = 0;

							ZeroMemory(buffer, BUFFER_SIZE);

							bytesReceived = recv(currentSocket, &buffer[0], BUFFER_SIZE, 0);

							if (bytesReceived > 0)
							{
								if (this->MessageReceived != NULL)
								{
									for (unsigned int j = 0; j < m_clients.size(); j++)
									{
										if (currentSocket != m_clients[j].socket)
										{
											this->MessageReceived(this, m_clients[j].socket, std::string(buffer, 0, bytesReceived));
										}
									}
								}
							}
							else
							{
								std::cout << "Client disconnected!" << std::endl;
								FD_CLR(currentSocket, &master);
								closesocket(currentSocket);

								for (unsigned int j = 0; j < m_clients.size(); j++)
								{
									if (currentSocket == m_clients[j].socket)
									{
										m_clients.erase(m_clients.begin() + j);
									}

								}
							}
						}
					}
				}
			}
			closesocket(m_socket);
		}

		void Server::Send(const int& clientSocket, const std::string& msg)
		{
			send(clientSocket, msg.c_str(), msg.size() + 1, 0);
		}
	}
}