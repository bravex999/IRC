#include "../includes/IrcServer.hpp"
#include "../includes/Replies.hpp"
#include <sstream>
#include <cstdlib>

void IrcServer::_cmdPass(UserConn *c, const std::string &args)
{
	if (c->isRegistered())
	{
		_send(c->getFd(), ERR_ALREADYREGISTERED);
		return;
	}
	if (args.empty())
	{
		_send(c->getFd(), ERR_NEEDMOREPARAMS("PASS"));
		return;
	}
	if (args != _pass)
	{
		_send(c->getFd(), ERR_PASSWDMISMATCH);
		return;
	}
	c->setHasPass(true);
}

void IrcServer::_cmdNick(UserConn *c, const std::string &args)
{
	if (args.empty())
	{
		_send(c->getFd(), ERR_NEEDMOREPARAMS("NICK"));
		return;
	}
	if (!c->hasPass())
		return;

	if (_getClient(args))
	{
		_send(c->getFd(), ERR_NICKNAMEINUSE(args));
		return;
	}

	std::string oldNick = c->getNick();
	c->setNick(args);

	if (c->isRegistered())
	{
		std::string msg = FMT_NICK(oldNick, c->getUser(), c->getIp(), args);
		_send(c->getFd(), msg);
		for (size_t i = 0; i < _channels.size(); ++i)
		{
			if (_channels[i]->isMember(c))
				_broadcast(msg, _channels[i], c->getFd());
		}
	}
	else if (!c->getUser().empty())
	{
		c->setRegistered(true);
		_send(c->getFd(), RPL_WELCOME(c->getNick(), c->getUser(), c->getIp()));
	}
}

void IrcServer::_cmdUser(UserConn *c, const std::string &args)
{
	if (c->isRegistered())
	{
		_send(c->getFd(), ERR_ALREADYREGISTERED);
		return;
	}
	if (args.empty())
	{
		_send(c->getFd(), ERR_NEEDMOREPARAMS("USER"));
		return;
	}
	if (!c->hasPass())
		return;

	std::stringstream ss(args);
	std::string user;
	std::string mode;
	std::string unused;
	std::string real;

	ss >> user >> mode >> unused;
	if (user.empty() || mode.empty() || unused.empty())
	{
		_send(c->getFd(), ERR_NEEDMOREPARAMS("USER"));
		return;
	}		
	std::getline(ss, real);
	if (!real.empty() && real[0] == ' ')
		real.erase(0, 1);
	if (!real.empty() && real[0] == ':')
		real.erase(0, 1);

	c->setUser(user);
	c->setReal(real);

	if (!c->getNick().empty())
	{
		c->setRegistered(true);
		_send(c->getFd(), RPL_WELCOME(c->getNick(), c->getUser(), c->getIp()));
	}
}

void IrcServer::_cmdJoin(UserConn *c, const std::string &args)
{
	if (args.empty())
	{
		_send(c->getFd(), ERR_NEEDMOREPARAMS("JOIN"));
		return;
	}

	std::stringstream ss(args);
	std::string name;
	std::string key;
	ss >> name >> key;

	if (name.empty() || name[0] != '#')
		return;

	ChatRoom *chan = _getChannel(name);
	if (!chan)
	{
		chan = _createChannel(name);
		chan->addAdmin(c);
	}
	else
	{
		if (chan->isMember(c))
			return;
		if (chan->isInviteOnly() && !c->hasInvite(name))
		{
			_send(c->getFd(), ERR_INVITEONLYCHAN(name));
			return;
		}
		if (!chan->getKey().empty() && chan->getKey() != key)
		{
			_send(c->getFd(), ERR_BADCHANNELKEY(name));
			return;
		}
		if (chan->isFull())
		{
			_send(c->getFd(), ERR_CHANNELISFULL(name));
			return;
		}
	}

	chan->addMember(c);
	if (c->hasInvite(name))
		c->removeInvite(name);

	std::string joinMsg = FMT_JOIN(c->getNick(), c->getUser(), c->getIp(), name);
	_broadcast(joinMsg, chan, -1);

	if (!chan->getTopic().empty())
		_send(c->getFd(), RPL_TOPIC(c->getNick(), name, chan->getTopic()));
	else
		_send(c->getFd(), RPL_NOTOPIC(c->getNick(), name));

	_send(c->getFd(), RPL_NAMREPLY(c->getNick(), name, chan->getUserListStr()));
	_send(c->getFd(), RPL_ENDOFNAMES(c->getNick(), name));
}

void IrcServer::_cmdPart(UserConn *c, const std::string &args)
{
	std::stringstream ss(args);
	std::string name;
	std::string reason;
	ss >> name;
	if (name.empty())
	{
		_send(c->getFd(), ERR_NEEDMOREPARAMS("PART"));
		return;
	}
	std::getline(ss, reason);
	if (!reason.empty() && reason[0] == ' ')
		reason.erase(0, 1);
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);

	ChatRoom *chan = _getChannel(name);
	if (!chan)
	{
		_send(c->getFd(), ERR_NOSUCHCHANNEL(name));
		return;
	}
	if (!chan->isMember(c))
	{
		_send(c->getFd(), ERR_NOTONCHANNEL(name));
		return;
	}

	std::string msg = FMT_PART(c->getNick(), c->getUser(), c->getIp(), name, reason);
	_broadcast(msg, chan, -1);
	chan->removeMember(c);

	if (chan->getMembers().empty())
	{
		for (size_t i = 0; i < _channels.size(); ++i)
		{
			if (_channels[i] == chan)
			{
				_channels.erase(_channels.begin() + i);
				delete chan;
				break;
			}
		}
	}
}

void IrcServer::_cmdKick(UserConn *c, const std::string &args)
{
	std::stringstream ss(args);
	std::string name;
	std::string target;
	std::string reason;

	ss >> name >> target;
	std::getline(ss, reason);
	if (!reason.empty() && reason[0] == ' ')
		reason.erase(0, 1);
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);

	ChatRoom *chan = _getChannel(name);
	if (!chan)
	{
		_send(c->getFd(), ERR_NOSUCHCHANNEL(name));
		return;
	}
	if (!chan->isMember(c))
	{
		_send(c->getFd(), ERR_NOTONCHANNEL(name));
		return;
	}
	if (!chan->isAdmin(c))
	{
		_send(c->getFd(), ERR_CHANOPRIVSNEEDED(name));
		return;
	}

	UserConn *t = _getClient(target);
	if (!t || !chan->isMember(t))
	{
		_send(c->getFd(), ERR_USERNOTINCHANNEL(target, name));
		return;
	}

	std::string msg = FMT_KICK(c->getNick(), c->getUser(), c->getIp(), name, target, reason);
	_broadcast(msg, chan, -1);
	chan->removeMember(t);
}

void IrcServer::_cmdPrivMsg(UserConn *c, const std::string &args)
{
	std::stringstream ss(args);
	std::string target;
	std::string msg;
	ss >> target;
	std::getline(ss, msg);
	if (!msg.empty() && msg[0] == ' ')
		msg.erase(0, 1);
	if (!msg.empty() && msg[0] == ':')
		msg.erase(0, 1);
	if (target.empty())
	{
		_send(c->getFd(), ERR_NEEDMOREPARAMS("PRIVMSG"));
		return;
	}
	if (msg.empty())
	{
		_send(c->getFd(), ERR_NOTEXTTOSEND);
		return;
	}
	if (target[0] == '#')
	{
		ChatRoom *chan = _getChannel(target);
		if (!chan)
		{
			_send(c->getFd(), ERR_NOSUCHCHANNEL(target));
			return;
		}
		if (!chan->isMember(c))
		{
			_send(c->getFd(), ERR_CANNOTSENDTOCHAN(target));
			return;
		}
		_broadcast(FMT_PRIVMSG(c->getNick(), c->getUser(), c->getIp(), target, msg), chan, c->getFd());
	}
	else
	{
		UserConn *dest = _getClient(target);
		if (!dest)
		{
			_send(c->getFd(), ERR_NOSUCHNICK(target));
			return;
		}
		_send(dest->getFd(), FMT_PRIVMSG(c->getNick(), c->getUser(), c->getIp(), target, msg));
	}
}

void IrcServer::_cmdTopic(UserConn *c, const std::string &args)
{
	std::stringstream ss(args);
	std::string name;
	std::string topic;

	ss >> name;
	std::getline(ss, topic);

	ChatRoom *chan = _getChannel(name);
	if (!chan)
	{
		_send(c->getFd(), ERR_NOSUCHCHANNEL(name));
		return;
	}
	if (!chan->isMember(c))
	{
		_send(c->getFd(), ERR_NOTONCHANNEL(name));
		return;
	}

	if (topic.empty())
	{
		if (chan->getTopic().empty())
			_send(c->getFd(), RPL_NOTOPIC(c->getNick(), name));
		else
			_send(c->getFd(), RPL_TOPIC(c->getNick(), name, chan->getTopic()));
	}
	else
	{
		if (!topic.empty() && topic[0] == ' ')
			topic.erase(0, 1);
		if (!topic.empty() && topic[0] == ':')
			topic.erase(0, 1);

		if (chan->isTopicRestricted() && !chan->isAdmin(c))
		{
			_send(c->getFd(), ERR_CHANOPRIVSNEEDED(name));
			return;
		}
		chan->setTopic(topic);
		_broadcast(FMT_TOPIC(c->getNick(), c->getUser(), c->getIp(), name, topic), chan, -1);
	}
}

void IrcServer::_cmdInvite(UserConn *c, const std::string &args)
{
	std::stringstream ss(args);
	std::string target;
	std::string name;

	ss >> target >> name;

	if (name.empty())
	{
		_send(c->getFd(), ERR_NEEDMOREPARAMS("INVITE"));
		return;
	}

	ChatRoom *chan = _getChannel(name);
	if (!chan)
	{
		_send(c->getFd(), ERR_NOSUCHCHANNEL(name));
		return;
	}
	if (!chan->isMember(c))
	{
		_send(c->getFd(), ERR_NOTONCHANNEL(name));
		return;
	}
	if (!chan->isAdmin(c))
	{
		_send(c->getFd(), ERR_CHANOPRIVSNEEDED(name));
		return;
	}

	UserConn *dest = _getClient(target);
	if (!dest)
	{
		_send(c->getFd(), ERR_NOSUCHNICK(target));
		return;
	}
	if (chan->isMember(dest))
	{
		_send(c->getFd(), ERR_USERONCHANNEL(target, name));
		return;
	}

	dest->addInvite(name);
	_send(c->getFd(), RPL_INVITING(c->getNick(), target, name));
	_send(dest->getFd(), FMT_INVITE(c->getNick(), c->getUser(), c->getIp(), target, name));
}

void IrcServer::_cmdMode(UserConn *c, const std::string &args)
{
	std::stringstream ss(args);
	std::string name;
	std::string modes;

	ss >> name >> modes;

	ChatRoom *chan = _getChannel(name);
	if (!chan)
	{
		_send(c->getFd(), ERR_NOSUCHCHANNEL(name));
		return;
	}

	if (modes.empty())
	{
		_send(c->getFd(), RPL_MODE(c->getNick(), name, chan->getModeString(), ""));
		return;
	}

	if (!chan->isAdmin(c))
	{
		_send(c->getFd(), ERR_CHANOPRIVSNEEDED(name));
		return;
	}

	std::string resultModes;
	std::string resultArgs;
	bool add = true;

	for (size_t i = 0; i < modes.length(); ++i)
	{
		char mode = modes[i];
		if (mode == '+')
		{
			add = true;
			resultModes += "+";
		}
		else if (mode == '-')
		{
			add = false;
			resultModes += "-";
		}
		else if (mode == 'i')
		{
			chan->setInviteMode(add);
			resultModes += "i";
		}
		else if (mode == 't')
		{
			chan->setTopicMode(add);
			resultModes += "t";
		}
		else if (mode == 'k')
		{
			std::string key;
			ss >> key;
			if (add)
				chan->setKey(key);
			else
				chan->setKey("");
			resultModes += "k";
			if (!key.empty())
			{
				resultArgs += key;
				resultArgs += " ";
			}
		}
		else if (mode == 'o')
		{
			std::string target;
			ss >> target;
			UserConn *t = _getClient(target);
			if (!t)
			{
				_send(c->getFd(), ERR_NOSUCHNICK(target));
				continue;
			}
			if (!chan->isMember(t))
			{
				_send(c->getFd(), ERR_USERNOTINCHANNEL(target, name));
				continue;
			}
			if (add)
				chan->addAdmin(t);
			else
				chan->removeAdmin(t);
			resultModes += "o";
			resultArgs += target;
			resultArgs += " ";
		}
		else if (mode == 'l')
		{
			if (add)
			{
				std::string limitStr;
				ss >> limitStr;
				chan->setLimit(std::atoi(limitStr.c_str()));
				resultModes += "l";
				resultArgs += limitStr;
				resultArgs += " ";
			}
			else
			{
				chan->setLimit(0);
				resultModes += "l";
			}
		}
		else
		{
			_send(c->getFd(), ERR_UNKNOWNMODE(mode));
			return;
		}
	}

	if (!resultModes.empty())
		_broadcast(FMT_MODE(c->getNick(), c->getUser(), c->getIp(), name, resultModes, resultArgs), chan, -1);
}

void IrcServer::_cmdQuit(UserConn *c, const std::string &args)
{
	std::string msg = args;
	if (msg.empty())
		msg = "Quit";
	if (!msg.empty() && msg[0] == ':')
		msg.erase(0, 1);

	_removeClient(c->getFd(), msg);
}

