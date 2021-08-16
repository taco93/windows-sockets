#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_tcp_server.h"
#include "net_udp_server.h"

namespace network
{
	namespace udp
	{
		void SendTo(Server* server, struct sockaddr* client, const socklen_t& len, const std::string& msg)
		{
			server->Send(client, len, msg);
		}
	}

	namespace tcp
	{
		void Send(network::tcp::Server* server, const int& client, const std::string& msg)
		{
			server->Send(client, msg);
		}
	}
}