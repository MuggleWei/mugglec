## network

mugglec网络模块部分.  

我并不打算一开始就讲述如何使用mugglec中的网络模块, 而是会从一些简单的socket应用开始, 一步一步学习如何使用socket, 以及为什么需要使用封装后的网络库.  

### 简单的socket用法

#### 有缺陷的TCP Echo服务
让我们从一个简单的Linux下的TCP echo服务开始吧: [IPv4 TCP Echo Server v1](../codes/network_unix/ip4_tcp_echo_srv_v1/ip4_tcp_echo_srv_v1.c), 很容易发现, 这个服务有着明显的缺陷, 一次只能处理一个连接, 直到前一个连接断开之后, 才会去处理后面的连接. 我们可不想一次只服务一个连接, 使用断点调试一下会发现, 处理逻辑是卡在了读入socket消息当中, 那么是否可以在没有消息的情况下, 就忽略当前连接, 紧接着去处理其他连接呢? 答案是肯定的!  

#### 糟糕的非阻塞忙轮询
细心的朋友可能已经发现, 我们在send消息的时候, 设置了标志MSG_DONTWAIT, 它表示如果写缓冲区满的时候, 我们并不会一直等待下去. 同理, 我们也可以将recv和accept的socket设置为非阻塞的模式, 从而在一个连接没有消息需要读取时, 去处理其他的连接(完整代码 [IPv4 TCP Echo Server v2](../codes/network_unix/ip4_tcp_echo_srv_v2/ip4_tcp_echo_srv_v2.c)):
```

...

void* handle_client(socket_peer_t *peer)
{
    ...

    while (1)
    {
        num_read = recv(peer->fd, buf, sizeof(buf), 0);
        if (num_read > 0)
        {
            ...
        }
        else
        {
            if (num_read == -1)
            {
                if (errno == EINTR)
                {
                    ...
                }
                else if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 使用非阻塞模式时, 当没有读消息需要处理时, 直接返回去处理别的消息
                    fprintf(stdout, "client would block: addr=%s\n", peer->straddr);
                    return peer;
                }
                else
                {
                    ...
                }
            }
            else if (num_read == 0)
            {
                ...
            }
        }
    }

    ...
}
...

int main(int argc, char *argv[])
{
    ...

    if (listen(lfd, 16) < 0)
    {
        fprintf(stderr, "failed listen: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 设置socket为非阻塞模式
    if (set_socket_nonblock(lfd) == -1)
    {
        fprintf(stderr, "failed set socket nonblocking: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        ...

        peer->fd = accept(lfd, (struct sockaddr*)&peer->addr, &peer->addrlen);
        if (peer->fd == -1)
        {
            if (errno == EINTR)
            {
                ...
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 非阻塞模式时, 没有等待接入的连接
            }
            else
            {
                ...
            }
        }

        ...
    }

    ...
}


```

使用多个telnet连接上来, 发现当前我们确实可以一次性处理多个连接啦! 但是还是有些不对, 怎么日志在飞速的刷呢, 再用top看一下, 发现CPU占比相当的高, 这太糟了, 明明只做了很少的事情, 却疯狂的占用计算机的CPU资源, 这可不是我们想要的.  

#### 多线程
已经发现, 非阻塞忙轮询并不是一个好的解决方案, 那么我们要如何改进这个服务呢? 有一些编程基础的朋友这时可能马上会想到线程. 当一个连接建立时, 我们就开一个新的线程来处理这个连接, 这样我们便可以一次性处理多个连接了, 且在没有消息的时候, 不用无意义的消耗CPU资源. 啊哈~ 那就让我们着手来修改一下最开始的程序吧(完整的代码 [IPv4 TCP Echo Server v3](../codes/network_unix/ip4_tcp_echo_srv_v3/ip4_tcp_echo_srv_v3.c)):
```

...

#include "muggle/c/muggle_c.h"  // 包含mugglec的头文件, 这样可以方便的使用mugglec中已经封装好的线程函数

...

int main(int argc, char *argv[])
{
    ...

    while (1)
    {
        ...

        // 替换之前的直接调用, 改为开一条新的线程来调用

        // // handle client
        // handle_client(peer);

        // create thread and handle client
        muggle_thread_t th;
        muggle_thread_create(&th, handle_client, (void*)peer);
        muggle_thread_detach(&th);

        ...
    }

    ...
}
                


```

经过简单的替换, 我们已经可以轻松的同时处理多个连接上来的客户端了, 并且CPU资源占用也不高. 所以这就完成了吗? 哈哈哈, 当然没有, 当连接数量少时, 这种方案似乎没有问题, 但是当连接数量成百上千时, 为每个连接创建一个单独的线程的开销便显得过于昂贵了.  

那还能有什么其他解法呢? 让我们回到问题本身, 让看看我们需要解决到事情
* 有多个连接, 且不能因为某个连接阻塞而影响其他连接
* 用较小到开销, 检查多个socket文件描述符, 看它们中是否有哪个可以执行IO操作

### I/O多路复用与异步I/O
I/O多路复用允许同时检查多个文件描诉符, 发现它们当中是否有哪个可以执行I/O操作, 准确的说, 是检查它们的I/O调用是否可以非阻塞的执行. Unix系统下, select和poll是比较通用API, 而各个平台也有自己效率更高的专属API, 比如Linux的epoll, FreeBSD的kqueue, Solaris的/dev/null等.  

Windows虽然没有提供专属的I/O多路复用API, 但提供了一个专属的异步I/O: IOCP(IO Completion Ports) API.  

那么这就通过select API来改写一下上面到程序吧, 并且顺便更改一下代码, 可以同时支持IP协议的不同版本(完整的代码 [TCP Echo Server Use Select](../codes/network_unix/tcp_echo_srv_select/tcp_echo_srv_select.c)). 让我们看一下用户逻辑部分的代码, 其实关注的只是连接, 消息到达, 断开以及定时器:
```
void on_connection(socket_peer_t *peer)
{
	fprintf(stdout, "on_connection: %s\n", peer->straddr);
}

void on_message(socket_peer_t *peer, char *buf, ssize_t num_bytes)
{
...
}

void on_timer()
{
	fprintf(stdout, "timer...\n");
}

void on_error(socket_peer_t *peer)
{
	fprintf(stdout, "on_error: %s\n", peer->straddr);
}
```
可以看到, 用户处理逻辑的代码只占很少到一部分, 而大部分代码实际是在处理select API的逻辑, 以及一些异常的处理, 比如处理EINTR. 而且连用户的那么一点代码当中, 也有大部分是在处理写失败的情况. 这还仅仅在使用select, 如果你还想根据平台, 选择使用poll, epoll或是iocp, 或是有时需要使用不同到协议, 比如UDP, QUIC或者在内网进行组播来传送消息, 那么直接使用操作系统相关的API会是十分复杂到一件事情.  

### 使用mugglec网络模块
如果用户想要专注于开发应用层的协议和应用逻辑, 避免把精力过多的分散到关注底层的细节中去, 可以使用现成的网络库. 不同的语言都有十分优秀且使用上类似的网络库存在, 比如c有libevent, libuv, Java有netty; 这些网络库有众多的用户, 也十分容易上手, 是开发网络应用的首选. 当然如果你并不想开发自己的私有协议, 而是想使用一些众所周知的协议, 比如web应用常用的http, websocket, 或是一些rpc调用, 那么使用一些开箱即用的库或是框架会是更好的选择.  

mugglec也带有自己的网络模块, 其主要目标并不是实现众所周知的一些协议, 而是封装底层的细节, 便于灵活的使用和开发自定义的协议  

#### 简单的TCP Echo服务
那么现在就让我们使用mugglec的网络模块, 来构建一个简单的TCP Echo服务吧(完整的代码 [Echo Server V1](../codes/network_simple/echo_srv_v1/echo_srv_v1.c))
```
void on_connect(struct muggle_socket_event *ev, struct muggle_socket_peer *listen_peer, struct muggle_socket_peer *peer) { ... }
void on_error(struct muggle_socket_event *ev, struct muggle_socket_peer *peer) { ... }
void on_close(struct muggle_socket_event *ev, struct muggle_socket_peer *peer) { ... }
void on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer) { ... }

...

int main(int argc, char *argv[])
{
	...

	// 创建TCP监听socket peer
	const char *host = argv[1];
	const char *serv = argv[2];

	muggle_socket_peer_t peer;
	if (muggle_tcp_listen(host, serv, 512, &peer) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create tcp listen for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	// 填充初始化参数
	muggle_socket_event_init_arg_t ev_init_arg;
	memset(&ev_init_arg, 0, sizeof(ev_init_arg));
	ev_init_arg.cnt_peer = 1;
	ev_init_arg.peers = &peer;
	ev_init_arg.on_connect = on_connect;
	ev_init_arg.on_error = on_error;
	ev_init_arg.on_close = on_close;
	ev_init_arg.on_message = on_message;

	// 消息循环
	muggle_socket_event_t ev;
	if (muggle_socket_event_init(&ev_init_arg, &ev) != 0)
	{
		MUGGLE_LOG_ERROR("failed init socket event");
		exit(EXIT_FAILURE);
	}
	muggle_socket_event_loop(&ev);

	return 0;
}

```

整个代码编写十分简单, 总结起来就是分为三步:  
* 创建用于监听的socket peer
* 填充socket event的初始化参数, 填入需要加入消息循环中的socket peer, 回调函数等(当然还有许多可用的参数, 之后会慢慢介绍)
* 初始化socket event, 并且运行

#### Echo服务
刚刚我们一直在使用TCP服务, 现在让我们也把UDP的Echo加入到程序中来吧(完整的代码 [Echo Server V2](../codes/network_simple/echo_srv_v2/echo_srv_v2.c))
```

void on_message(struct muggle_socket_event *ev, struct muggle_socket_peer *peer)
{
	while (1)
	{
		if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_UDP_PEER)
		{
			...
		}
		else if (peer->peer_type == MUGGLE_SOCKET_PEER_TYPE_TCP_PEER)
		{
			...
		}

		...
	}
}

int main(int argc, char *argv[])
{
	...

	muggle_socket_peer_t peers[2];

	// 创建TCP监听socket peer
	if (muggle_tcp_listen(host, serv, 512, &peers[0]) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create tcp listen for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	// 创建UDP socket peer并绑定
	if (muggle_udp_bind(host, serv, &peers[1]) == MUGGLE_INVALID_SOCKET)
	{
		MUGGLE_LOG_ERROR("failed create udp bind for %s:%s", host, serv);
		exit(EXIT_FAILURE);
	}

	muggle_socket_event_init_arg_t ev_init_arg;
	memset(&ev_init_arg, 0, sizeof(ev_init_arg));

	// 指定填入两个socket peer
	ev_init_arg.cnt_peer = 2;
	ev_init_arg.peers = peers;

	...
}

```
稍加改动, 就可以加入UDP的echo服务, 当然, 真实的服务大多只会提供单一协议的服务, 这里作为一个例子展示.  

### 自定义协议
到现在为止, 我们已经看到一些简单的例子, 当然真实的服务并不会如此简单, 比如TCP会遇到粘包, UDP需要自己处理丢包与重传, 还有需要传输自定义的消息而不是单纯的字符串. mugglec提供了一些简单的工具, 可以在自定义协议时, 提供便利.  

#### 编解码与字节流缓存
现在让我们试着来自定义一个简单私有协议, 它基于TCP, 每个消息包都带有一个简单的包头, 定义如下:
```
|<---         head       --->|<---  body   --->|
| message type | body length |    body bytes   |
|    4 bytes   |   4 bytes   | variable length |
```

* 包头前4字节是用户自定义的消息类型
* 包头后4字节, 用于指明body的长度
* body是消息中携带的数据

按照上面的消息设定, 我们开始编写一个名为Foo的服务, 服务接受客户端发送过来的登录消息, 返回登录结果, 另外服务还定时向所有连接发送心跳消息(完整的代码 服务器[Foo Server](../codes/network_foo_v1/foo_srv/foo_srv.c), 客户端[Foo Client](../codes/network_foo_v1/foo_cli/foo_cli.c)):
