#include <crtdbg.h>
#include <network.h>

class Server : public network::server_interface<MessageType>
{
private:

public:
	// Inherited via server_interface
	virtual void OnClientConnect() override
	{
		std::cout << "[SERVER] Client has connected!" << std::endl;
	}

	virtual void OnClientDisconnect() override
	{
		std::cout << "[SERVER] Client has disconnected!" << std::endl;
	}

	virtual void OnMessageReceived() override
	{
	}
};

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//network::tcp::Server myServer("127.0.0.1", 4950, network::tcp::Send);

	//if (myServer.Init())
	//{
	//	myServer.Run();
	//}

	Server s;
	s.Start(4950);
	s.Update();

	return 0;
}	