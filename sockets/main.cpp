#include <crtdbg.h>
#include <network.h>
#include <Windows.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//network::tcp::Server myServer("127.0.0.1", 4950, 
	//	[](network::tcp::Server* server, const int& client, const std::string& msg) { server->Send(client, msg); });

	network::udp::Server myServer("127.0.0.1", 4950,
		[](network::udp::Server* server, struct sockaddr* client, const socklen_t& len, const std::string& msg) { server->Send(client, len, msg); });

	if (myServer.Init())
	{
		myServer.Run();
	}

	return 0;
}	