#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

void ipv4_ntop(const char *ip_str, uint16_t port)
{
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));

	int ret = inet_pton(AF_INET, ip_str, (void*)&sa.sin_addr);
	ASSERT_EQ(ret, 1);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);

	char buf[MUGGLE_SOCKET_ADDR_STRLEN];
	const char *addr_str = muggle_socket_ntop(
		(struct sockaddr*)&sa, buf, sizeof(buf), 0);
	ASSERT_TRUE(addr_str != NULL);

	char raw[MUGGLE_SOCKET_ADDR_STRLEN];
	snprintf(raw, sizeof(raw), "%s:%d", ip_str, (int)port);
	ASSERT_STREQ(raw, buf);
}

void ipv6_ntop(const char *ip_str, uint16_t port)
{
	struct sockaddr_in6 sa;
	memset(&sa, 0, sizeof(sa));

	int ret = inet_pton(AF_INET6, ip_str, (void*)&sa.sin6_addr);
	ASSERT_EQ(ret, 1);

	sa.sin6_family = AF_INET6;
	sa.sin6_port = htons(port);

	char buf[MUGGLE_SOCKET_ADDR_STRLEN];
	const char *addr_str = muggle_socket_ntop(
		(struct sockaddr*)&sa, buf, sizeof(buf), 0);
	ASSERT_TRUE(addr_str != NULL);

	char raw[MUGGLE_SOCKET_ADDR_STRLEN];
	snprintf(raw, sizeof(raw), "[%s]:%d", ip_str, (int)port);
	ASSERT_STREQ(raw, buf);
}

TEST(socket_ntop, ipv4)
{
	ipv4_ntop("127.0.0.1", 0);
	ipv4_ntop("127.0.0.1", 8080);
	ipv4_ntop("8.8.8.8", 8888);
}

TEST(socket_ntop, ipv6)
{
	ipv6_ntop("::1", 0);
	ipv6_ntop("::1", 8080);
	ipv6_ntop("2001:4860:4860::8888", 8888);
	ipv6_ntop("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 8888);
}
