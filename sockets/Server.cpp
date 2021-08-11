#include "Server.h"

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
}

Server::~Server()
{
	WSACleanup();
}

void Server::Run()
{
	SOCKET listening = CreateSocket();

	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Failed to create a socket error code: " << WSAGetLastError() << std::endl;
		return;
	}

	fd_set master = {};
	FD_SET(listening, &master);
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
				if (currentSocket == listening)
				{
					ClientMetaData client = {};
					client.socket = WaitForConnection(listening, &client);

					if (client.socket == INVALID_SOCKET)
					{
						return;
					}
					inet_ntop(client.addr.sin_family, &client.addr.sin_addr, &client.ipAsString[0], IP_LEN);
					std::cout << "Client connected from: " << client.ipAsString << ":" << ntohs(client.addr.sin_port) << std::endl;
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
	closesocket(listening);
}

void Server::Send(const int& clientSocket, const std::string& msg)
{
	send(clientSocket, msg.c_str(), msg.size() + 1, 0);
}