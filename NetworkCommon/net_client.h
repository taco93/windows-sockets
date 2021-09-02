#pragma once
#include "net_connection.h"

namespace network
{
	template <typename T>
	class client_interface
	{
	private:
		std::unique_ptr<connection<T>> m_connection;

	public:
		client_interface()
		{

		}

		virtual ~client_interface()
		{
			Disconnect();
		}

	public:
		// Given IP and port establish a connection to the server
		bool Connect(const std::string& ip, const uint16_t port);

		// Disconnect from the server
		void Disconnect();

		// Check to see if client is connected to a server
		bool IsConnected();
	};

	template<typename T>
	inline bool client_interface<T>::Connect(const std::string& ip, const uint16_t port)
	{
		m_connection = std::make_unique<connection<T>>();

		if (m_connection->ConnectToServer(ip, port) == false)
		{
			return false;
		}

		return true;
	}

	template<typename T>
	inline void client_interface<T>::Disconnect()
	{
		if (IsConnected())
		{
			if (m_connection->Disconnect())
			{
				m_connection.release();
			}
		}
	}

	template<typename T>
	inline bool client_interface<T>::IsConnected()
	{
		if (m_connection)
		{
			return m_connection->IsConnected();
		}
		else
		{
			return false;
		}
	}
}