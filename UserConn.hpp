#ifndef USERCONN_HPP
#define USERCONN_HPP

#include <string>
#include <vector>

class UserConn
{
private:
	int							_fd;
	std::string					_ip;
	std::string					_nick;
	std::string					_user;
	std::string					_real;
	std::string					_buffer;
	std::vector<std::string>	_invites;
	std::string					_out;
	bool						_hasPass;
	bool						_isRegistered;

public:
	UserConn(int fd, const std::string &ip);
	~UserConn();

	int			getFd() const;
	std::string	getIp() const;
	std::string	getNick() const;
	std::string	getUser() const;
	std::string	getReal() const;
	std::string	getBuffer() const;
	bool		isRegistered() const;
	bool		hasPass() const;
	std::string	getFullName() const;

	void		setNick(const std::string &n);
	void		setUser(const std::string &u);
	void		setReal(const std::string &r);
	void		setRegistered(bool status);
	void		setHasPass(bool status);

	void		appendBuffer(const std::string &data);
	void		clearBuffer();

	void		addInvite(const std::string &chan);
	void		removeInvite(const std::string &chan);
	bool		hasInvite(const std::string &chan) const;

	void		queueOut(const std::string &msg);
	bool		hasOutData() const;
	std::string	&getOutBuffer();
	const std::string	&getOutBuffer() const;
	void		consumeOut(size_t n);
};

#endif

