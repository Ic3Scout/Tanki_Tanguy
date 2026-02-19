#include <iostream>

#include "pch.h"
#include "Server.h"

int main()
{
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);

	Server server;
	server.Start();
	server.MainLoop();

    return 0;
}