#include <network.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	network::Client client("127.0.0.1", 4950);

	if (client.Init())
	{
		client.Run();
	}

	return 0;
}