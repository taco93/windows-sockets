#pragma once
#include "net_common.h"
#include <functional>

namespace network
{
	class server_interface
	{
	private:
		SOCKET m_socket;
		std::function<void(const uint16_t&, const std::string&)> MessageReceivedHandler;
		std::function<void(const std::string&, const uint16_t&)> ClientConnectHandler;
		std::function<void()> ClientDisconnectHandler;

		SOCKET WaitForConnection();
		std::string PrintSocketData(struct addrinfo* p);

	private:
		SOCKET CreateSocket(const uint16_t& port);

	public:
		server_interface()
		{
			m_socket = INVALID_SOCKET;

			using namespace std::placeholders;
			MessageReceivedHandler = std::bind(&server_interface::OnMessageReceived, this, _1, _2);
			ClientConnectHandler = std::bind(&server_interface::OnClientConnect, this, _1, _2);
			ClientDisconnectHandler = std::bind(&server_interface::OnClientDisconnect, this);
		}
		virtual ~server_interface()
		{
			WSACleanup();
		}

		bool Start(const uint16_t& port);
		void Update();
		void Stop();

	public:
		virtual void OnClientConnect(const std::string& ip, const uint16_t& port) = 0;
		virtual void OnClientDisconnect() = 0;
		virtual void OnMessageReceived(const uint16_t& socketId, const std::string& msg) = 0;
	};

	inline SOCKET server_interface::WaitForConnection()
	{
		return accept(m_socket, nullptr, nullptr);
	}

	inline std::string server_interface::PrintSocketData(addrinfo* p)
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
			data += "Socktype: TCP\n";
		}
		else if (p->ai_socktype == SOCK_DGRAM)
		{
			data += "Socktype: UDP\n";
		}

		char ipAsString[IPV6_ADDRSTRLEN] = {};

		inet_ntop(p->ai_family, &((struct sockaddr_in*)p->ai_addr)->sin_addr, ipAsString, sizeof(ipAsString));

		data += "Host: ";
		data += ipAsString;
		data += ":";
		data += std::to_string(GetPort(p->ai_addr));

		data += "\n";

		return data;
	}

	inline SOCKET server_interface::CreateSocket(const uint16_t& port)
	{
		// Get a linked network structure based on provided hints
		struct addrinfo hints, * servinfo, * p;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		int8_t rv = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &servinfo);

		if (rv != 0)
		{
			std::cerr << "Addrinfo: " << gai_strerror(rv) << std::endl;
			return INVALID_SOCKET;
		}

		SOCKET listener;

		// Loop through linked list of possible network structures
		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
			std::cout << PrintSocketData(p) << std::endl;

			listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

			if (listener == INVALID_SOCKET)
			{
				continue;
			}
			if (bind(listener, p->ai_addr, p->ai_addrlen) != 0)
			{
				std::cerr << "Bind error code: " << WSAGetLastError() << std::endl;
				closesocket(listener);
				continue;
			}

			break;
		}

		// Reached end of list and could not connect to any
		if (p == nullptr)
		{
			return INVALID_SOCKET;
		}

		freeaddrinfo(servinfo);

		return listener;
	}

	inline bool server_interface::Start(const uint16_t& port)
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

		m_socket = CreateSocket(port);

		u_long enable = 1;
		setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&enable), sizeof(enable));
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable));

		if (listen(m_socket, SOMAXCONN) == INVALID_SOCKET)
		{
			std::cerr << "Listen:" << WSAGetLastError() << std::endl;
			return false;
		}

		return true;
	}

	inline void server_interface::Update()
	{
		fd_set master = {};
		FD_SET(m_socket, &master);
		char buffer[BUFFER_SIZE];

		while (1)
		{
			ZeroMemory(buffer, sizeof(buffer));
			fd_set copy = master;

			int8_t socketCount = select(0, &copy, nullptr, nullptr, nullptr);

			if (socketCount == SOCKET_ERROR)
			{
				std::cout << "Select: " << WSAGetLastError() << std::endl;
			}

			for (size_t i = 0; i < (size_t)socketCount; i++)
			{
				SOCKET currentSocket = copy.fd_array[i];

				if (FD_ISSET(currentSocket, &copy))
				{
					if (currentSocket == m_socket)
					{
						SOCKET currentSocket = WaitForConnection();

						if (currentSocket != INVALID_SOCKET)
						{
							if (this->ClientConnectHandler != NULL)
							{
								FD_SET(currentSocket, &master);
								char ipAsString[IPV6_ADDRSTRLEN] = {};
								struct sockaddr_in client = {};
								socklen_t len = sizeof(client);
								getpeername(currentSocket, (struct sockaddr*)&client, &len);
								inet_ntop(client.sin_family, &client.sin_addr, ipAsString, sizeof(ipAsString));

								this->ClientConnectHandler(std::string(ipAsString), GetPort((struct sockaddr*)&client));
							}
							continue;
						}
					}
					else
					{
						int32_t bytes = recv(currentSocket, buffer, sizeof(buffer), 0);

						if (bytes > 0)
						{
							if (this->MessageReceivedHandler != NULL)
							{
								this->MessageReceivedHandler(currentSocket, std::string(buffer, bytes + 1));
							}
						}
						else
						{
							if (errno == WSAEWOULDBLOCK)
							{
								continue;
							}
							else
							{
								if (this->ClientDisconnectHandler != NULL)
								{
									FD_CLR(currentSocket, &master);
									this->ClientDisconnectHandler();
								}
							}
						}
					}
				}
			}
		}
	}

	inline void server_interface::Stop()
	{

	}
}