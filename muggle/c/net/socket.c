#include "socket.h"
#include <string.h>

int muggle_socket_lib_init()
{
#if MUGGLE_PLATFORM_WINDOWS
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
	return 0;
#endif
}

muggle_socket_t muggle_socket_create(int family, int type, int protocol)
{
	return socket(family, type, protocol);
}

int muggle_socket_close(muggle_socket_t socket)
{
#if MUGGLE_PLATFORM_WINDOWS
	return closesocket(socket);
#else
	return close(socket);
#endif
}

int muggle_socket_strerror(int errnum, char *buf, size_t bufsize)
{
#if MUGGLE_PLATFORM_WINDOWS
	DWORD ret = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM, NULL, errnum,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			buf, (DWORD)bufsize, 0);

	return ret > 0 ? 0 : -1;
#else
	return strerror_r(errnum, buf, bufsize);
#endif
}
