#include <network.h>

int main()
{
	network::Client client("127.0.0.1", 4950);

	if (client.Init())
	{
		client.Run();
	}

	getchar();
	return 0;
}