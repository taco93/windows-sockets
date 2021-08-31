#pragma once

#include "net_common.h"
#include "net_message.h"

namespace network
{
	template <typename T>
	struct buffer
	{
		std::pair<uint8_t*, size_t> data;

		buffer(message<T>& msg)
		{
			data.second = msg.size();

			data.first = new uint8_t[data.second];
			for (size_t i = 0; i < msg.payload.size(); i++)
			{
				std::cout << msg.payload[i] << " ";
			}
			std::cout << "\n";

			memcpy(data.first, &msg.header, sizeof(msg.header));
			memcpy(data.first + sizeof(msg.header), msg.payload.data(), msg.payload.size());

		};
	};
}