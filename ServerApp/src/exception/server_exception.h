
#pragma once

/**
 * includes.
 */
#include <exception>
#include <string>
#include <cstring>

/**
 * ServerException class definition.
 */
class ServerException: public std::exception {
private:
	std::string msg;

public:
	ServerException(void) : exception() {
		msg = "";
	}

	ServerException(std::string error) : exception(), msg(error) {
		if (error.empty() != true) {
			this->msg = error;
		} else {
			msg = "";
		}
	}

	ServerException(int err) : exception() {
		switch (err) {
		case EACCES:
			this->msg = "The process does not have appropriate privileges.";
			break;
		case EADDRINUSE:
			this->msg = "The specified address is already in use.";
			break;
		case EADDRNOTAVAIL:
			this->msg = "The specified address is not available from the local machine.";
			break;
		case EALREADY:
			this->msg = "A connection request is already in progress for the specified socket.";
			break;
		case EAFNOSUPPORT:
			this->msg = "The implementation does not support the specified address family.";
			break;
		/*****/
		case EBADF:
			this->msg = "The socket argument is not a valid file descriptor.";
			break;
		/*****/
		case ECONNABORTED:
			this->msg = "A connection has been aborted.";
			break;
		case ECONNREFUSED:
			this->msg = "The target address was not listening for connections or refused the connection request.";
			break;
		case ECONNRESET:
			this->msg = "Remote host reset the connection request.";
			break;
		/*****/
		case EDESTADDRREQ:
		case EISDIR:
			this->msg = "The address argument is a null pointer.";
			break;
		/*****/
		case EIO:
			this-> msg = "An I/O error occurred.";
			break;
		case EINPROGRESS:
			this->msg = "O_NONBLOCK is set for the file descriptor for the socket and the connection cannot be immediately established; the connection will be established asynchronously.";
			break;
		/*****/
		case EFAULT:
			this->msg = "The address or address_len parameter can not be accessed or written.";
			break;
		/*****/
		case EHOSTUNREACH:
			this->msg = "The destination host cannot be reached (probably because the host is down or a remote router cannot reach it).";
			break;
		/*****/
		case EINTR:
			this->msg = "The accept() function was interrupted by a signal that was caught before a valid connection arrived.";
			break;
		case EINVAL:
			this->msg = "The socket is not accepting connections.";
			break;
		case EISCONN:
			this->msg = "The socket is already connected.";
			break;
		/*****/
		case ELOOP:
			this->msg = "Too many symbolic links were encountered in translating the pathname in address.";
			break;
		/*****/
		case ENAMETOOLONG:
			this->msg = "A component of a pathname exceeded {NAME_MAX} characters, or an entire pathname exceeded {PATH_MAX} characters.";
			break;
		case ENETUNREACH:
			this->msg = "No route to the network is present.";
			break;
		case ENETDOWN:
			this->msg = "The local interface used to reach the destination is down.";
			break;
		case ENOTDIR:
			this->msg = "A component of the path prefix of the pathname in address is not a directory.";
			break;
		case ENOBUFS: /* No buffer space available. */
			this->msg = "Insufficient resources were available to complete the call.";
			break;
		case ENOENT:
			this->msg = "A component of the pathname does not name an existing file or the pathname is an empty string.";
			break;
		case ENOTSOCK:
			this->msg = "The socket argument does not refer to a socket.";
			break;
		case ENFILE:
			this->msg = "No more file descriptors are available for the system.";
			break;
		case ENOMEM:
			this->msg = "Insufficient memory was available to fulfill the request.";
			break;
		case ENOSR:
			this->msg = "There were insufficient STREAMS resources available for the operation to complete.";
			break;
		/*****/
		case EOPNOTSUPP:
			this->msg = "The socket type of the specified socket does not support accepting connections.";
			break;
		/*****/
		case EPROTONOSUPPORT:
			this->msg = "The protocol is not supported by the address family, or the protocol is not supported by the implementation.";
			break;
		case EPROTOTYPE:
			this->msg = "The socket type is not supported by the protocol.";
			break;
		case EPROTO:
			this->msg = "A protocol error has occurred; for example, the STREAMS protocol stack has not been initialised.";
			break;
		/*****/
		case EROFS:
			this->msg = "The name would reside on a read-only filesystem.";
			break;
		/*****/
		case ETIMEDOUT:
			this->msg = "The attempt to connect timed out before a connection was made.";
			break;
		/*****/
		case EWOULDBLOCK: /* EAGAIN */
			this->msg = "O_NONBLOCK is set for the socket file descriptor and no connections are present to be accepted.";
			break;
		default:
			this->msg = "Unknown error code + " + err;
		}
	}

	ServerException(const ServerException& srvex) {
		this->msg = srvex.msg;
	}

	~ServerException() {
	}

	const char* what() const throw(){
		if (msg.empty() == true) {
			return "";
		} else {
			return msg.c_str();
		}
	}
};
