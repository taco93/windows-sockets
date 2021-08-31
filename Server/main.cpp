#include <crtdbg.h>
#include <network.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	network::udp::Server myServer("193.11.184.20", 4950, network::udp::SendTo);

	if (myServer.Init())
	{
		myServer.Run();
	}

	return 0;
}	