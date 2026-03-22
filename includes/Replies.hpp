#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

#define CRLF "\r\n"

#define RPL_WELCOME(nick, user, host) (std::string(":Server 001 ") + nick + " :Welcome " + nick + "!" + user + "@" + host + CRLF)
#define RPL_NAMREPLY(nick, chan, users) (std::string(":Server 353 ") + nick + " = " + chan + " :" + users + CRLF)
#define RPL_ENDOFNAMES(nick, chan) (std::string(":Server 366 ") + nick + " " + chan + " :End of /NAMES list" + CRLF)
#define RPL_TOPIC(nick, chan, topic) (std::string(":Server 332 ") + nick + " " + chan + " :" + topic + CRLF)
#define RPL_NOTOPIC(nick, chan) (std::string(":Server 331 ") + nick + " " + chan + " :No topic is set" + CRLF)
#define RPL_INVITING(nick, target, chan) (std::string(":Server 341 ") + nick + " " + target + " " + chan + CRLF)
#define RPL_MODE(nick, chan, modes, args) (std::string(":Server 324 ") + nick + " " + chan + " " + modes + " " + args + CRLF)

#define FMT_JOIN(nick, user, host, chan) (std::string(":") + nick + "!" + user + "@" + host + " JOIN " + chan + CRLF)
#define FMT_PART(nick, user, host, chan, msg) (std::string(":") + nick + "!" + user + "@" + host + " PART " + chan + " :" + msg + CRLF)
#define FMT_KICK(nick, user, host, chan, target, msg) (std::string(":") + nick + "!" + user + "@" + host + " KICK " + chan + " " + target + " :" + msg + CRLF)
#define FMT_PRIVMSG(nick, user, host, target, msg) (std::string(":") + nick + "!" + user + "@" + host + " PRIVMSG " + target + " :" + msg + CRLF)
#define FMT_MODE(nick, user, host, chan, mode, arg) (std::string(":") + nick + "!" + user + "@" + host + " MODE " + chan + " " + mode + " " + arg + CRLF)
#define FMT_INVITE(nick, user, host, target, chan) (std::string(":") + nick + "!" + user + "@" + host + " INVITE " + target + " " + chan + CRLF)
#define FMT_QUIT(nick, user, host, msg) (std::string(":") + nick + "!" + user + "@" + host + " QUIT :" + msg + CRLF)
#define FMT_NICK(oldnick, user, host, newnick) (std::string(":") + oldnick + "!" + user + "@" + host + " NICK " + newnick + CRLF)

#define ERR_NOSUCHNICK(nick) (std::string(":Server 401 * ") + nick + " :No such nick/channel" + CRLF)
#define ERR_NOSUCHCHANNEL(chan) (std::string(":Server 403 * ") + chan + " :No such channel" + CRLF)
#define ERR_CANNOTSENDTOCHAN(chan) (std::string(":Server 404 * ") + chan + " :Cannot send to channel" + CRLF)
#define ERR_UNKNOWNCOMMAND(cmd) (std::string(":Server 421 * ") + cmd + " :Unknown command" + CRLF)
#define ERR_NICKNAMEINUSE(nick) (std::string(":Server 433 * ") + nick + " :Nickname is already in use" + CRLF)
#define ERR_USERNOTINCHANNEL(nick, chan) (std::string(":Server 441 ") + nick + " " + chan + " :They aren't on that channel" + CRLF)
#define ERR_NOTONCHANNEL(chan) (std::string(":Server 442 * ") + chan + " :You're not on that channel" + CRLF)
#define ERR_USERONCHANNEL(user, chan) (std::string(":Server 443 ") + user + " " + chan + " :is already on channel" + CRLF)
#define ERR_NOTREGISTERED (std::string(":Server 451 * :You have not registered") + CRLF)
#define ERR_NEEDMOREPARAMS(cmd) (std::string(":Server 461 * ") + cmd + " :Not enough parameters" + CRLF)
#define ERR_ALREADYREGISTERED (std::string(":Server 462 * :Unauthorized command (already registered)") + CRLF)
#define ERR_PASSWDMISMATCH (std::string(":Server 464 * :Password incorrect") + CRLF)
#define ERR_CHANNELISFULL(chan) (std::string(":Server 471 * ") + chan + " :Cannot join channel (+l)" + CRLF)
#define ERR_UNKNOWNMODE(ch) (std::string(":Server 472 * ") + std::string(1, ch) + " :is unknown mode char" + CRLF)
#define ERR_INVITEONLYCHAN(chan) (std::string(":Server 473 * ") + chan + " :Cannot join channel (+i)" + CRLF)
#define ERR_BADCHANNELKEY(chan) (std::string(":Server 475 * ") + chan + " :Cannot join channel (+k)" + CRLF)
#define ERR_CHANOPRIVSNEEDED(chan) (std::string(":Server 482 * ") + chan + " :You're not channel operator" + CRLF)
#define ERR_ERRONEUSNICKNAME(nick) (std::string(":Server 432 * ") + nick + " :Erroneous nickname" + CRLF)
#define FMT_TOPIC(nick, user, host, chan, topic) (std::string(":") + nick + "!" + user + "@" + host + " TOPIC " + chan + " :" + topic + CRLF)
#define ERR_NOTEXTTOSEND (std::string(":Server 412 * :No text to send") + CRLF)

#endif

