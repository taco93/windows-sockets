#include <crtdbg.h>
#include <network.h>
#include <Windows.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	network::udp::Server myServer("127.0.0.1", 4950, network::udp::SendTo);

	if (myServer.Init())
	{
		myServer.Run();
	}

	return 0;
}	