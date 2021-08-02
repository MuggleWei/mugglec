- [network](#network)
  - [简单的socket用法](#简单的socket用法)
    - [有缺陷的TCP Echo服务](#有缺陷的tcp-echo服务)
    - [糟糕的非阻塞忙轮询](#糟糕的非阻塞忙轮询)
    - [多线程](#多线程)
  - [TODO:](#todo)

## network

mugglec网络模块部分.  

我并不打算一开始就讲述如何使用mugglec中的网络模块, 而是会从一些简单的socket应用开始, 一步一步学习如何使用socket, 以及为什么需要使用封装后的网络库.  

### 简单的socket用法

#### 有缺陷的TCP Echo服务
让我们从一个简单的Linux下的TCP echo服务开始吧: [IPv4 TCP Echo Server v1](../codes/network/ip4_tcp_echo_srv_v1.c), 可以看到, 这个服务有着明显的缺陷, 一次只能处理一个连接, 直到前一个连接断开之后, 才会去处理后面的连接. 我们可不想一次只服务一个连接, 使用断点调试一下会发现, 处理逻辑是卡在了读入socket消息当中, 那么是否可以在没有消息的情况下, 就忽略当前连接, 紧接着去处理其他连接呢? 答案是肯定的!  

#### 糟糕的非阻塞忙轮询
细心的朋友可能已经发现, 我们在send消息的时候, 设置了标志MSG_DONTWAIT, 它表示如果写缓冲区满的时候, 我们并不会一直等待下去. 同理, 我们也可以将recv和accept的socket设置为非阻塞的模式(完整代码 [IPv4 TCP Echo Server v2](../codes/network/ip4_tcp_echo_srv_v2.c)):
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
已经发现, 非阻塞忙轮询并不是一个好的解决方案, 那么我们要如何改进这个服务呢? 有一些编程基础的朋友这时可能马上会想到线程. 当一个连接建立时, 我们就开一个新的线程来处理这个连接, 这样我们便可以一次性处理多个连接了, 且在没有消息的时候, 不用无意义的消耗CPU资源. 啊哈~ 那就让我们着手来修改一下最开始的程序吧(完整的代码 [IPv4 Tcp Echo Server v3](../codes/network/ip4_tcp_echo_srv_v3.c)):
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

可以看到, 经过简单的替换, 我们已经可以轻松的同时处理多个连接上来的客户端了, 并且CPU资源占用也不高. 所以这就完成了吗? 哈哈哈, 当然没有, 如果这就算最终答案, 那么libevent, libuv, netty之类的代码库也许就不会出现了. 当连接数量少时, 这种方案似乎没有问题, 但是当连接数量成百上千时, 为每个连接创建一个单独的线程便显不是那么有效率了.  

### TODO: