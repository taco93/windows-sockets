#include <network.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	network::udp::Client client("193.11.184.20", 4950);

	if (client.Init())
	{
		client.Run();
	}

	return 0;
}