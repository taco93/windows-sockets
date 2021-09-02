#include <network.h>

class Client : public network::client_interface<MessageType>
{
public:

private:

};

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Client c;

	c.Connect("127.0.0.1", 4950);

	if (c.IsConnected())
	{
		std::cout << "I am connected!" << std::endl;
	}
	c.Disconnect();

	if (!c.IsConnected())
	{
		std::cout << "I am disconnected!" << std::endl;
	}

	getchar();

	return 0;
}