/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChatRoom.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chnaranj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 11:49:15 by chnaranj          #+#    #+#             */
/*   Updated: 2026/03/23 11:49:17 by chnaranj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHATROOM_HPP
#define CHATROOM_HPP

#include "UserConn.hpp"
#include <vector>
#include <string>

class ChatRoom
{
private:
	std::string				_name;
	std::string				_topic;
	std::string				_key;
	int						_limit;
	bool					_inviteMode;
	bool					_topicMode;
	std::vector<UserConn*>	_members;
	std::vector<UserConn*>	_admins;

public:
	ChatRoom(const std::string &name);
	~ChatRoom();

	std::string				getName() const;
	std::string				getTopic() const;
	std::string				getKey() const;
	int						getLimit() const;
	bool					isInviteOnly() const;
	bool					isTopicRestricted() const;
	std::string				getModeString() const;
	std::string				getUserListStr() const;
	std::vector<UserConn*>	getMembers();

	void	setTopic(const std::string &t);
	void	setKey(const std::string &k);
	void	setLimit(int l);
	void	setInviteMode(bool on);
	void	setTopicMode(bool on);

	void	addMember(UserConn *u);
	void	removeMember(UserConn *u);
	void	addAdmin(UserConn *u);
	void	removeAdmin(UserConn *u);

	bool	isMember(UserConn *u) const;
	bool	isAdmin(UserConn *u) const;
	bool	isFull() const;
};

#endif
