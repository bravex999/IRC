int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	std::string portStr = argv[1];
	for (size_t i = 0; i < portStr.length(); ++i)
	{
		if (portStr[i] < '0' || portStr[i] > '9')
		{
			std::cerr << "Error: invalid port" << std::endl;
			return 1;
		}
	}

	int port = std::atoi(argv[1]);
	if (port < 1 || port > 65535)
	{
		std::cerr << "Error: port out of range (1-65535)" << std::endl;
		return 1;
	}

	IrcServer server(port, argv[2]);
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
