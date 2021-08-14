#include <crtdbg.h>
#include <network.h>
#include <Windows.h>

enum class MessageType
{
	FIRE_BULLET,
	CAT_DOG
};

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	network::Server myServer("127.0.0.1", 4950, 
		[](network::Server* server, const int& client, const std::string& msg) { server->Send(client, msg); });

	if (myServer.Init())
	{
		myServer.Run();
	}

	return 0;
}	