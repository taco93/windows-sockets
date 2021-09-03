#include <crtdbg.h>
#include <network.h>

class Server : public network::server_interface
{
private:

public:
	Server()
	{
	}

	// Inherited via server_interface
	virtual void OnClientConnect() override
	{
		std::cout << "[SERVER] Client has connected!" << std::endl;
	}

	virtual void OnClientDisconnect() override
	{
		std::cout << "[SERVER] Client has disconnected!" << std::endl;
	}

	virtual void OnMessageReceived(const uint16_t& socketId, const std::string& msg) override
	{
		std::cout << msg << std::endl;
	}
};

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Server s;
	s.Start(4950);
	s.Update();

	return 0;
}