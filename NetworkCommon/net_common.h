#pragma once

#include <mutex>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <ws2tcpip.h>


#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif