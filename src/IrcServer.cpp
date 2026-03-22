#include "../includes/IrcServer.hpp"
#include "../includes/Replies.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <csignal>
#include <cstdlib>

bool IrcServer::_running = true;

IrcServer::IrcServer(int port, const std::string &pass)
: _port(port), _pass(pass), _serverSocket(-1)
{
}

IrcServer::~IrcServer()
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		close(_clients[i]->getFd());
		delete _clients[i];
	}
	for (size_t i = 0; i < _channels.size(); ++i)
		delete _channels[i];
	if (_serverSocket != -1)
		close(_serverSocket);
}

void IrcServer::_signalHandler(int signum)
{
	(void)signum;
	_running = false;
}

void IrcServer::start()
{
	_initSocket();
	std::signal(SIGINT, _signalHandler);
	std::signal(SIGQUIT, _signalHandler);
	std::signal(SIGPIPE, SIG_IGN);
	_runLoop();
}

void IrcServer::_initSocket()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
		throw std::runtime_error("Socket failed");

	int opt = 1;
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	fcntl(_serverSocket, F_SETFL, O_NONBLOCK);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	if (bind(_serverSocket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
		throw std::runtime_error("Bind failed");
	if (listen(_serverSocket, SOMAXCONN) < 0)
		throw std::runtime_error("Listen failed");

	pollfd pfd;
	pfd.fd = _serverSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollFds.push_back(pfd);
	std::cout << "Server listening on " << _port << std::endl;
}

pollfd *IrcServer::_getPollFd(int fd)
{
	for (size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd == fd)
			return &_pollFds[i];
	}
	return 0;
}

void IrcServer::_compactPollFds()
{
	for (size_t i = 0; i < _pollFds.size();)
	{
		if (_pollFds[i].fd == -1)
			_pollFds.erase(_pollFds.begin() + i);
		else
			++i;
	}
}

void IrcServer::_runLoop()
{
	while (_running)
	{
		if (poll(&_pollFds[0], _pollFds.size(), -1) < 0 && _running)
			throw std::runtime_error("Poll failed");

		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			int fd = _pollFds[i].fd;
			short rev = _pollFds[i].revents;

			if (fd < 0)
				continue;

			if (rev & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (fd != _serverSocket)
					_removeClient(fd, "Quit");
				continue;
			}

			if (rev & POLLIN)
			{
				if (fd == _serverSocket)
					_acceptClient();
				else
					_handleData(fd);
			}

			if (rev & POLLOUT)
			{
				if (fd != _serverSocket)
					_flushOut(fd);
			}
		}
		_compactPollFds();
	}
}

void IrcServer::_acceptClient()
{
	sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int fd = accept(_serverSocket, reinterpret_cast<struct sockaddr*>(&addr), &len);
	if (fd < 0)
		return;

	fcntl(fd, F_SETFL, O_NONBLOCK);
	pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollFds.push_back(pfd);
	_clients.push_back(new UserConn(fd, inet_ntoa(addr.sin_addr)));
	std::cout << "Client connected: " << fd << std::endl;
}

void IrcServer::_handleData(int fd)
{
	char buf[1024];
	int bytes = recv(fd, buf, sizeof(buf) - 1, 0);

	if (bytes == 0)
	{
		_removeClient(fd, "Quit");
		return;
	}
	if (bytes < 0)
		return;
	buf[bytes] = 0;
	UserConn *c = _getClient(fd);
	if (!c)
		return;

	c->appendBuffer(buf);
	std::string data = c->getBuffer();
	size_t pos;

	while ((pos = data.find("\n")) != std::string::npos)
	{
		std::string line = data.substr(0, pos);
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
		_exec(line, fd);
		if (!_getClient(fd))
			return;
		data.erase(0, pos + 1);
	}
	c->clearBuffer();
	c->appendBuffer(data);
}

void IrcServer::_removeClient(int fd, const std::string &reason)
{
	UserConn *c = _getClient(fd);
	if (!c)
		return;

	if (c->isRegistered() && !c->getNick().empty())
	{
		std::string quitMsg = FMT_QUIT(c->getNick(), c->getUser(), c->getIp(), reason);
		for (size_t i = 0; i < _channels.size(); ++i)
		{
			if (_channels[i]->isMember(c))
				_broadcast(quitMsg, _channels[i], fd);
		}
	}

	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->isMember(c))
		{
			_channels[i]->removeMember(c);
			if (_channels[i]->getMembers().empty()) // if the channel is empty, the channel must be destroy
			{
				ChatRoom *tmp = _channels[i];
				_channels.erase(_channels.begin() + i);
				delete tmp;
				--i;
			}
		}
	}

	pollfd *pfd = _getPollFd(fd);
	if (pfd)
	{
		pfd->fd = -1;
		pfd->events = 0;
		pfd->revents = 0;
	}

	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i] == c)
		{
			_clients.erase(_clients.begin() + i);
			break;
		}
	}

	close(fd);
	delete c;
	std::cout << "Client disconnected: " << fd << std::endl;
}

UserConn* IrcServer::_getClient(int fd)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getFd() == fd)
			return _clients[i];
	}
	return 0;
}

UserConn* IrcServer::_getClient(const std::string &nick)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getNick() == nick)
			return _clients[i];
	}
	return 0;
}

ChatRoom* IrcServer::_getChannel(const std::string &name)
{
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->getName() == name)
			return _channels[i];
	}
	return 0;
}

ChatRoom* IrcServer::_createChannel(const std::string &name)
{
	ChatRoom *c = new ChatRoom(name);
	_channels.push_back(c);
	return c;
}

void IrcServer::_send(int fd, const std::string &msg)
{
	UserConn *c = _getClient(fd);
	if (!c)
		return;

	c->queueOut(msg);

	pollfd *pfd = _getPollFd(fd);
	if (pfd)
		pfd->events |= POLLOUT;
}

void IrcServer::_flushOut(int fd)
{
	UserConn *c = _getClient(fd);
	if (!c)
		return;

	std::string &out = c->getOutBuffer();
	if (out.empty())
		return;

	ssize_t sent = send(fd, out.c_str(), out.length(), 0);
	if (sent > 0)
		c->consumeOut(static_cast<size_t>(sent));
	else if (sent == 0)
	{
		_removeClient(fd, "Quit");
		return;
	}
	else
		return;
	pollfd *pfd = _getPollFd(fd);
	if (pfd && !c->hasOutData())
		pfd->events &= ~POLLOUT;
}

void IrcServer::_broadcast(const std::string &msg, ChatRoom *chan, int excludeFd)
{
	std::vector<UserConn*> members = chan->getMembers();
	for (size_t i = 0; i < members.size(); ++i)
	{
		if (members[i]->getFd() != excludeFd)
			_send(members[i]->getFd(), msg);
	}
}

void IrcServer::_exec(const std::string &line, int fd)
{
	if (line.empty())
		return;

	std::stringstream ss(line);
	std::string cmd;
	ss >> cmd;
	std::string args;
	std::getline(ss, args);
	if (!args.empty() && args[0] == ' ')
		args.erase(0, 1);

	UserConn *c = _getClient(fd);
	if (!c)
		return;

	if (cmd == "PASS") _cmdPass(c, args);
	else if (cmd == "NICK") _cmdNick(c, args);
	else if (cmd == "USER") _cmdUser(c, args);
	else if (cmd == "QUIT") _cmdQuit(c, args);
	else if (cmd == "PING")
	{
		std::string token = args;
		if (!token.empty() && token[0] == ':')
			token.erase(0, 1);
		if (token.empty())
			token = "Server";
		_send(fd, std::string(":Server PONG Server :") + token + CRLF);
	}
	else if (cmd == "PONG")
	{
		(void)args;
	}
	else if (!c->isRegistered()) _send(fd, ERR_NOTREGISTERED);
	else if (cmd == "JOIN") _cmdJoin(c, args);
	else if (cmd == "PART") _cmdPart(c, args);
	else if (cmd == "KICK") _cmdKick(c, args);
	else if (cmd == "INVITE") _cmdInvite(c, args);
	else if (cmd == "TOPIC") _cmdTopic(c, args);
	else if (cmd == "MODE") _cmdMode(c, args);
	else if (cmd == "PRIVMSG") _cmdPrivMsg(c, args);
	else if (cmd == "NOTICE" || cmd == "WHO" || cmd == "WHOIS" || cmd == "USERHOST" || cmd == "CAP")
	(void)args;
else _send(fd, ERR_UNKNOWNCOMMAND(cmd));
}
