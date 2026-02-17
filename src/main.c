#include "../includes/IrcServer.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	IrcServer server(std::atoi(argv[1]), argv[2]);
	try
	{
		server.start();
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}

