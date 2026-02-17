#include "../includes/ChatRoom.hpp"

ChatRoom::ChatRoom(const std::string &name)
: _name(name), _limit(0), _inviteMode(false), _topicMode(false)
{
}

ChatRoom::~ChatRoom()
{
}

std::string ChatRoom::getName() const
{
	return _name;
}

std::string ChatRoom::getTopic() const
{
	return _topic;
}

std::string ChatRoom::getKey() const
{
	return _key;
}

int ChatRoom::getLimit() const
{
	return _limit;
}

bool ChatRoom::isInviteOnly() const
{
	return _inviteMode;
}

bool ChatRoom::isTopicRestricted() const
{
	return _topicMode;
}

void ChatRoom::setTopic(const std::string &t)
{
	_topic = t;
}

void ChatRoom::setKey(const std::string &k)
{
	_key = k;
}

void ChatRoom::setLimit(int l)
{
	_limit = l;
}

void ChatRoom::setInviteMode(bool on)
{
	_inviteMode = on;
}

void ChatRoom::setTopicMode(bool on)
{
	_topicMode = on;
}

void ChatRoom::addMember(UserConn *u)
{
	if (!isMember(u))
		_members.push_back(u);
}

void ChatRoom::removeMember(UserConn *u)
{
	for (size_t i = 0; i < _members.size(); ++i)
	{
		if (_members[i] == u)
		{
			_members.erase(_members.begin() + i);
			break;
		}
	}
	removeAdmin(u);
	if (_admins.empty() && !_members.empty())
	{
		addAdmin(_members[0]);
	}
}

void ChatRoom::addAdmin(UserConn *u)
{
	if (!isAdmin(u))
		_admins.push_back(u);
}

void ChatRoom::removeAdmin(UserConn *u)
{
	for (size_t i = 0; i < _admins.size(); ++i)
	{
		if (_admins[i] == u)
		{
			_admins.erase(_admins.begin() + i);
			break;
		}
	}
}

bool ChatRoom::isMember(UserConn *u) const
{
	for (size_t i = 0; i < _members.size(); ++i)
	{
		if (_members[i] == u)
			return true;
	}
	return false;
}

bool ChatRoom::isAdmin(UserConn *u) const
{
	for (size_t i = 0; i < _admins.size(); ++i)
	{
		if (_admins[i] == u)
			return true;
	}
	return false;
}

bool ChatRoom::isFull() const
{
	return _limit > 0 && static_cast<int>(_members.size()) >= _limit;
}

std::vector<UserConn*> ChatRoom::getMembers()
{
	return _members;
}

std::string ChatRoom::getModeString() const
{
	std::string modes = "+";
	if (_inviteMode)
		modes += "i";
	if (_topicMode)
		modes += "t";
	if (!_key.empty())
		modes += "k";
	if (_limit > 0)
		modes += "l";
	return modes;
}

std::string ChatRoom::getUserListStr() const
{
	std::string list;
	for (size_t i = 0; i < _members.size(); ++i)
	{
		if (isAdmin(_members[i]))
			list += "@";
		list += _members[i]->getNick();
		list += " ";
	}
	return list;
}
