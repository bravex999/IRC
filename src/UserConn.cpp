/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UserConn.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chnaranj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 11:50:57 by chnaranj          #+#    #+#             */
/*   Updated: 2026/03/23 11:50:59 by chnaranj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/UserConn.hpp"

UserConn::UserConn(int fd, const std::string &ip)
: _fd(fd), _ip(ip), _hasPass(false), _isRegistered(false)
{
}

UserConn::~UserConn()
{
}

int UserConn::getFd() const
{
	return _fd;
}

std::string UserConn::getIp() const
{
	return _ip;
}

std::string UserConn::getNick() const
{
	return _nick;
}

std::string UserConn::getUser() const
{
	return _user;
}

std::string UserConn::getReal() const
{
	return _real;
}

std::string UserConn::getBuffer() const
{
	return _buffer;
}

bool UserConn::isRegistered() const
{
	return _isRegistered;
}

bool UserConn::hasPass() const
{
	return _hasPass;
}

std::string UserConn::getFullName() const
{
	return _nick + "!" + _user + "@" + _ip;
}

void UserConn::setNick(const std::string &n)
{
	_nick = n;
}

void UserConn::setUser(const std::string &u)
{
	_user = u;
}

void UserConn::setReal(const std::string &r)
{
	_real = r;
}

void UserConn::setRegistered(bool status)
{
	_isRegistered = status;
}

void UserConn::setHasPass(bool status)
{
	_hasPass = status;
}

void UserConn::appendBuffer(const std::string &data)
{
	_buffer += data;
}

void UserConn::clearBuffer()
{
	_buffer.clear();
}

void UserConn::addInvite(const std::string &chan)
{
	_invites.push_back(chan);
}

void UserConn::removeInvite(const std::string &chan)
{
	for (size_t i = 0; i < _invites.size(); ++i)
	{
		if (_invites[i] == chan)
		{
			_invites.erase(_invites.begin() + i);
			break;
		}
	}
}

bool UserConn::hasInvite(const std::string &chan) const
{
	for (size_t i = 0; i < _invites.size(); ++i)
	{
		if (_invites[i] == chan)
			return true;
	}
	return false;
}

void UserConn::queueOut(const std::string &msg)
{
	_out += msg;
}

bool UserConn::hasOutData() const
{
	return !_out.empty();
}

std::string &UserConn::getOutBuffer()
{
	return _out;
}

const std::string &UserConn::getOutBuffer() const
{
	return _out;
}

void UserConn::consumeOut(size_t n)
{
	if (n >= _out.size())
		_out.clear();
	else
		_out.erase(0, n);
}

