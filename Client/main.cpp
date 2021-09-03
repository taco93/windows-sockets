#include <network.h>

class Client : public network::client_interface<MessageType>
{
public:
	void Update()
	{
		bool key[3] = { false, false, false };
		bool old_key[3] = { false, false, false };

		while (1)
		{
			if (GetForegroundWindow() == GetConsoleWindow())
			{
				key[0] = GetAsyncKeyState('1') & 0x8000;
				key[1] = GetAsyncKeyState('2') & 0x8000;
				key[2] = GetAsyncKeyState('3') & 0x8000;
			}

			if (key[0] && !old_key[0])
			{
				network::message<MessageType> msg;

				Send(msg);
			}

			for (int i = 0; i < 3; i++)
			{
				old_key[i] = key[i];
			}
		}
	}

private:

};

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Client c;

	c.Connect("188.148.27.231", 4950);

	if (c.IsConnected())
	{
		std::cout << "I am connected!" << std::endl;

		c.Update();
	}
	c.Disconnect();

	if (!c.IsConnected())
	{
		std::cout << "I am disconnected!" << std::endl;
	}

	getchar();

	return 0;
}