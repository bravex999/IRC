*This project has been created as part of the 42 curriculum by chnaranj, abaldelo.*

## Description

ft_irc is an IRC (Internet Relay Chat) server written in C++98. It handles multiple clients simultaneously using a single poll() call for all I/O operations. Clients connect via TCP/IP, authenticate with a password, and can join channels, send private messages, and use operator commands.

Implemented features: authentication (PASS, NICK, USER), channel management (JOIN, PART, KICK, INVITE, TOPIC), messaging (PRIVMSG), and channel modes (i, t, k, o, l).

The reference client used for development and testing is HexChat.

## Instructions

Compile:

```
make
```

Run:

```
./ircserv <port> <password>
```

Example:

```
./ircserv 6667 mypass
```

Connect with HexChat: add a new network with server `127.0.0.1/6667`, set the server password, and connect.

Connect with netcat:

```
nc -C 127.0.0.1 6667
PASS mypass
NICK pepe
USER pepe 0 * :pepe
```

## Resources

- [RFC 2812](https://www.rfc-editor.org/rfc/rfc2812.html) — Official IRC Client Protocol specification. Used as the canonical reference for message formats, numeric replies, and nickname rules.
- [modern.ircdocs.horse](https://modern.ircdocs.horse/) — Modern and readable version of the IRC protocol. Used as the primary reference for command formats and practical examples.
- Various open-source IRC server implementations found online were reviewed as references for understanding protocol behavior and design patterns.

**AI usage:** AI tools were used for understanding concepts (non-blocking I/O, poll(), TCP buffering, IRC protocol details), explaining syscall behavior, reasoning about design decisions, and assisting with debugging.
