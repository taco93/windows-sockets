#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_tcp_server.h"
#include "net_udp_server.h"
#include "net_client.h"
#include "net_tcp_client.h"
#include "net_client.h"
#include "server_interface.h"

namespace network
{
	namespace udp
	{
		void SendTo(Server* server, struct sockaddr* client, const socklen_t& len, message<MessageType>& msg)
		{
			server->Send(client, len, msg);
		}
	}

	namespace tcp
	{
		void Send(Server* server, const int& client, const std::string& msg)
		{
			server->Send(client, msg);
		}
	}
}