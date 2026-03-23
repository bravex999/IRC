/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IrcServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chnaranj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 11:49:22 by chnaranj          #+#    #+#             */
/*   Updated: 2026/03/23 11:49:23 by chnaranj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <vector>
#include <string>
#include <poll.h>
#include "UserConn.hpp"
#include "ChatRoom.hpp"

class IrcServer
{
private:
	int						_port;
	std::string				_pass;
	int						_serverSocket;
	std::vector<pollfd>		_pollFds;
	std::vector<UserConn*>	_clients;
	std::vector<ChatRoom*>	_channels;
	static bool				_running;

	void	_initSocket();
	void	_runLoop();
	void	_acceptClient();
	void	_handleData(int fd);
	void	_removeClient(int fd, const std::string &reason = "Quit");

	void	_exec(const std::string &line, int fd);
	void	_send(int fd, const std::string &msg);
	void	_broadcast(const std::string &msg, ChatRoom *chan, int excludeFd);

	void	_flushOut(int fd);
	void	_compactPollFds();
	pollfd	*_getPollFd(int fd);

	UserConn*	_getClient(int fd);
	UserConn*	_getClient(const std::string &nick);
	ChatRoom*	_getChannel(const std::string &name);
	ChatRoom*	_createChannel(const std::string &name);

	void	_cmdPass(UserConn *c, const std::string &args);
	void	_cmdNick(UserConn *c, const std::string &args);
	void	_cmdUser(UserConn *c, const std::string &args);
	void	_cmdJoin(UserConn *c, const std::string &args);
	void	_cmdPart(UserConn *c, const std::string &args);
	void	_cmdKick(UserConn *c, const std::string &args);
	void	_cmdInvite(UserConn *c, const std::string &args);
	void	_cmdTopic(UserConn *c, const std::string &args);
	void	_cmdMode(UserConn *c, const std::string &args);
	void	_cmdPrivMsg(UserConn *c, const std::string &args);
	void	_cmdQuit(UserConn *c, const std::string &args);

	static void	_signalHandler(int signum);

public:
	IrcServer(int port, const std::string &pass);
	~IrcServer();
	void	start();
};

#endif
