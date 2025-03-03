- [Event \& Network](#event--network)
	- [事件模块](#事件模块)
		- [事件描述符](#事件描述符)
		- [事件上下文](#事件上下文)
		- [事件循环](#事件循环)
		- [直接使用Event来处理网络消息(不推荐)](#直接使用event来处理网络消息不推荐)
	- [网络模块](#网络模块)
		- [一个简单的Echo服务](#一个简单的echo服务)
		- [Socket事件回调](#socket事件回调)
			- [cb\_conn](#cb_conn)
			- [cb\_msg](#cb_msg)
			- [cb\_close](#cb_close)
			- [cb\_release](#cb_release)
			- [cb\_add\_ctx](#cb_add_ctx)
			- [cb\_wake](#cb_wake)
			- [cb\_timer](#cb_timer)
		- [简单的时间服务](#简单的时间服务)
		- [组播](#组播)
		- [多线程](#多线程)
		- [事件管道](#事件管道)
		- [自定义协议](#自定义协议)
			- [事件循环回调](#事件循环回调)
			- [协议头](#协议头)
			- [消息](#消息)
			- [编解码与消息分发](#编解码与消息分发)

# Event & Network
本节讲解**mugglec**中的事件与网络模块

## 事件模块

### 事件描述符
在Unix或Linux当中, stdin, stdout, 打开文件的句柄以及socket都是文件描述符, 他们可用于多路复用当中, 而在Windows中, 只有socket才可用于多路复用, 所以**mugglec**抽象了一个`muggle_event_fd`对象(注意, 不要与Linux当中的eventfd搞混了), 作为一个事件文件描述符.  

### 事件上下文
事件描述符`muggle_event_fd`仅仅是对操作系统提供的能够用于多路复用的一个文件描述符, **mugglec**当中还提供了实现上下文对象`muggle_event_context_t`, 方便用于事件处理

### 事件循环
当需要同时等待处理多个事件上下文时, 可以直接使用`muggle_event_loop_t`, 它提供了一个事件循环.  

### 直接使用Event来处理网络消息(不推荐)
例子[event_echo.c](./event_echo/event_echo.c) 展示了一个简单的使用事件循环的TCP Echo服务  

**初始化事件循环**
```
muggle_event_loop_init_args_t evloop_arg;
memset(&evloop_arg, 0, sizeof(evloop_arg));
evloop_arg.evloop_type = MUGGLE_EVLOOP_TYPE_NULL;
evloop_arg.hints_max_fd = 32;
evloop_arg.use_mem_pool = 1;

evloop_data_t *data = (evloop_data_t*)malloc(sizeof(evloop_data_t));
strncpy(data->msg, "Echo Server", sizeof(data->msg) - 1);

muggle_event_loop_t *evloop = muggle_evloop_new(&evloop_arg);
muggle_evloop_set_timer_interval(evloop, 3000);
muggle_evloop_set_cb_read(evloop, on_read);
muggle_evloop_set_cb_close(evloop, on_close);
muggle_evloop_set_cb_exit(evloop, on_exit);
muggle_evloop_set_data(evloop, data);
```
* `evloop_arg.hints_max_fd=32`: 提示事件循环会处理最多要处理的事件描述符数量
* `evloop_arg.use_mem_pool=1`: 设置事件循环内部的分配是否使用内存池, 若使用内存池, 则上面设置的`hints_max_fd`将会称为内存池的大小
* `muggle_evloop_set_cb_read`: 设置事件上下文当可读被触发时的回调函数
* `muggle_evloop_set_cb_close`: 设置事件上下文关闭的回调函数

创建了用于监听TCP连接的事件上下文, 并将其加入事件循环当中; 并且指定了这个上下文的附带数据, 在其中填入了TCP新来连接时的回调函数
```
muggle_socket_t tcp_listen_fd = muggle_tcp_listen(host, port, 512);
if (tcp_listen_fd == MUGGLE_INVALID_SOCKET)
{
	LOG_ERROR("failed TCP listen %s %s", host, port);
	exit(EXIT_FAILURE);
}
LOG_INFO("success TCP listen: %s %s", host, port);

ev_data_t tcp_ev_data;
memset(&tcp_ev_data, 0, sizeof(tcp_ev_data));
tcp_ev_data.cb_read = tcp_server_on_accept;

muggle_event_context_t tcp_ev_ctx;
muggle_ev_ctx_init(&tcp_ev_ctx, tcp_listen_fd, &tcp_ev_data);
if (muggle_evloop_add_ctx(evloop, &tcp_ev_ctx) != 0)
{
	LOG_ERROR("failed add listen socket context into event loop");
	exit(EXIT_FAILURE);
}
LOG_INFO("success insert listen socket into event loop");
```
* `muggle_tcp_listen`: 创建了TCP监听socket
* `tcp_ev_data.cb_read = tcp_server_on_accept`: 设置TCP监听到新来连接的回调函数
* `muggle_ev_ctx_init`: 初始化事件上下文, 将事件描述符和对应的数据装入
* `muggle_evloop_add_ctx`: 将事件上下文加入事件循环当中

**处理TCP连接**
```
void tcp_server_on_accept(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	do {
		muggle_socket_t fd = tcp_server_do_accept(evloop, ctx);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			break;
		}
		tcp_server_on_connect(evloop, fd);
	} while(1);
}
```
* `tcp_server_do_accept`: 用户获取新来的TCP socket描述符
* `tcp_server_on_connect`: 调用TCP新来连接的回调函数

```
void tcp_server_on_connect(muggle_event_loop_t *evloop, muggle_socket_t fd)
{
	LOG_INFO("on TCP connection");

	muggle_event_context_t *ctx =
		(muggle_event_context_t*)malloc(sizeof(muggle_event_context_t));

	ev_data_t *ev_data = (ev_data_t*)malloc(sizeof(ev_data_t));
	memset(ev_data, 0, sizeof(*ev_data));
	ev_data->cb_read = tcp_on_message;
	ev_data->cb_close = tcp_on_close;
	
	muggle_ev_ctx_init(ctx, fd, (void*)ev_data);
	int ret = muggle_evloop_add_ctx(evloop, (muggle_event_context_t*)ctx);
	if (ret != 0)
	{
		free(ev_data);
		free(ctx);
		muggle_socket_close(fd);
	}
}
```
* `ev_data->cb_read = tcp_on_message`: 指定事件上下文数据当中的cb_read, 之后用于此事件上下文的读回调函数
* `muggle_evloop_add_ctx`: 将此事件上下文加入事件循环当中

**回射TCP消息**
```
void tcp_on_message(muggle_event_loop_t *evloop, muggle_event_context_t *ctx)
{
	muggle_event_fd fd = muggle_ev_ctx_fd(ctx);

	char buf[1024];
	do {
		int n = muggle_ev_fd_read(fd, buf, sizeof(buf));
		if (n <= 0)
		{
			break;
		}

		if (muggle_ev_fd_write(fd, buf, n) != n)
		{
			muggle_ev_ctx_shutdown(ctx);
			break;
		}
	} while(1);
}
```

## 网络模块
在上一节当中, 我们直接使用`Event`模块来实现了一个TCP的Echo服务, 直接使用`Event`提供了足够的灵活性, 但是却需要用户处理许多繁杂的问题. **mugglec**当中的`Network`模块对网络操作提供了更方便的函数  

### 一个简单的Echo服务
让我们从最简单的Echo服务开始, 一步一步的展示如何使用**mugglec**当中的网络模块吧  
[echo_serv.c](./echo_serv/echo_serv.c) 是一个简单的Echo服务, 支持回射TCP和UDP的消息  

我们要做的归纳起来就三步
* 初始化事件循环
* 创建监听用的socket上下文, 并将其加入事件循环当中
* 处理消息

**初始化事件循环**
```
muggle_event_loop_init_args_t ev_init_args;
memset(&ev_init_args, 0, sizeof(ev_init_args));
ev_init_args.evloop_type = evloop_type;
ev_init_args.hints_max_fd = 32;
ev_init_args.use_mem_pool = 0;

muggle_event_loop_t *evloop = muggle_evloop_new(&ev_init_args);
if (evloop == NULL)
{
	LOG_ERROR("failed new event loop");
	exit(EXIT_FAILURE);
}
LOG_INFO("success new event loop");

// init socket event loop handle
muggle_socket_evloop_handle_t handle;
muggle_socket_evloop_handle_init(&handle);
muggle_socket_evloop_handle_set_cb_msg(&handle, on_message);
muggle_socket_evloop_handle_attach(&handle, evloop);
```
可以看到, 这里的初始化和直接使用`Event`来做网络事件循环的有一些不一样. 它不再直接往`muggle_event_loop_t`当中设置回调函数, 转而使用`muggle_socket_evloop_handle_t`作为网络事件循环的处理器, 而这个处理器当中, 只需要设置用户感兴趣的消息即可. 这里, 我们只关心远端发来消息的回调函数  
* `muggle_socket_evloop_handle_init`: 初始化网络事件循环的处理器
* `muggle_socket_evloop_handle_set_cb_msg`: 设置当消息触发时的回调函数
* `muggle_socket_evloop_handle_attach`: 将处理器放入事件循环当中

**将Socket上下文加入事件循环当中**
```
// create tcp listen socket
muggle_socket_context_t *tcp_listen_ctx = tcp_listen(host, serv);
muggle_socket_evloop_add_ctx(evloop, tcp_listen_ctx);

// create udp bind socket
muggle_socket_context_t *udp_bind_ctx = udp_bind(host, serv);
muggle_socket_evloop_add_ctx(evloop, udp_bind_ctx);
```
* `muggle_socket_evloop_add_ctx`: 将新生成的TCP和UDP监听的socket上下文加入事件循环当中

**处理消息**
```
void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	int n = 0;
	char buf[4096];
	if (ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT)
	{
		while ((n = muggle_socket_ctx_read(ctx, buf, sizeof(buf))) > 0)
		{
			muggle_socket_ctx_write(ctx, buf, n);
		}
	}
	else if (ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_UDP)
	{
		struct sockaddr_storage saddr;
		muggle_socklen_t len = sizeof(saddr);
		while ((n = muggle_socket_ctx_recvfrom(
			ctx, buf, sizeof(buf), 0, 
			(struct sockaddr*)&saddr, &len)) > 0)
		{
			muggle_socket_ctx_sendto(ctx, buf, n, 0, (struct sockaddr*)&saddr, len);
		}
	}
}
```
在on_message中, 我们根据`ctx->sock_type`的不同, 而决定如何回射消息  

用户可以直接使用`telnet`来试一下TCP消息的回射结果, **mugglec**的例子中也提供了一个方便的测试TCP/UDP回射服务的例子[net_str_client.c](./str_client//net_str_client.c), 可以使用它来测试上面写的Echo服务  

### Socket事件回调
上一小节, 我们使用`network`模块轻松的实现了一个同时支持TCP/UDP的Echo服务, 在例子当中, 我们只实现了`cb_msg(消息回调)`一个函数  
`muggle_socket_evloop_handle_t`提供了许多回调函数接口  

#### cb_conn
`cb_conn` 当一个TCP成功连接时的回调函数:
```
// 设置
muggle_socket_evloop_handle_set_cb_conn(handle, cb);

// 函数原型
void on_cb_conn(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
```
* `evloop`: 指向事件循环对象的指针
* `ctx`: 一个新连入的TCP `socket上下文`

#### cb_msg
`cb_msg` 当`socket上下文`有新来消息通知时的回调函数:
```
// 设置
muggle_socket_evloop_handle_set_cb_msg(handle, cb);

// 函数原型
void on_cb_msg(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
```
* `evloop`: 指向事件循环对象的指针
* `ctx`: 有新来消息通知的`socket上下文`

#### cb_close
`cb_close` 当`socket上下文`在事件循环当中被关闭时的回调函数:
```
// 设置
muggle_socket_evloop_handle_set_cb_close(handle, cb);

// 函数原型
void on_cb_close(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
```
* `evloop`: 指向事件循环对象的指针
* `ctx`: 在事件循环当中被关闭的`socket上下文`
* **注意: 当此`socket上下文`仅在此事件循环中被引用时, 此时`socket上下文`中的`data`资源可以被安全释放(但是不建议在此函数中释放); 若此`socket上下文`被多条线程引用时, 在此回调函数中释放`socket上下文`中的`data`资源是不合适的**

#### cb_release
`cb_release` 当`socket上下文`中的`data`资源可以在此事件循环中安全释放的回调函数:
```
// 设置
muggle_socket_evloop_handle_set_cb_release(handle, cb);

// 函数原型
void on_cb_release(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
```
* `evloop`: 指向事件循环对象的指针
* `ctx`: 可安全释放`data`资源的`socket上下文`
* **注意: 若`socket上下文`在事件循环被关闭时, 在其他线程还有引用, `cb_release`回调不被调用, 用户需要在合适的时机(`socket上下文`的引用计数降为0时), 自己控制`socket上下文`的释放**  

#### cb_add_ctx
`cb_add_ctx` 当`socket上下文`被加入到事件循环时的回调函数
```
// 设置
muggle_socket_evloop_handle_set_cb_add_ctx(handle, cb);

// 函数原型
void on_cb_add_ctx(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx);
```
* `evloop`: 指向事件循环对象的指针
* `ctx`: 被加入进事件循环的`socket上下文`

#### cb_wake
`cb_wake` 当事件循环被用户主动唤醒时的回调函数
```
// 设置
muggle_socket_evloop_handle_set_cb_wake(handle, cb);

// 函数原型
void on_cb_wake(muggle_event_loop_t *evloop);
```
* `evloop`: 指向事件循环对象的指针

#### cb_timer
`cb_timer` 当定时器触发时的回调函数
```
// 设置
muggle_socket_evloop_handle_set_timer_interval(handle, timeout);
muggle_socket_evloop_handle_set_cb_timer(handle, cb);

// 函数原型
void on_cb_timer(muggle_event_loop_t *evloop);
```
* `evloop`: 指向事件循环对象的指针
* `muggle_socket_evloop_handle_set_timer_interval`: 设置事件循环中的定时器触发时间间隔

### 简单的时间服务
上一小节提到了一些**Socket事件回调**, 那么现在我们就用一个简单的时间服务来展示一下这些回调函数的用法吧  
* 时间服务器: [time_serv.c](./time_serv/time_serv.c)
* 时间客户端: [time_client.c](./time_client/time_client.c)

每隔一段时间, 时间服务器就向已连接上的TCP客户端或指定的UDP或组播地址发送当前的时间字符串. 而客户端可选择TCP/UDP/组播模式, 从而接收从时间服务器发送出来的信息

### 组播
上一节我们同时展现了TCP/UDP/组播的使用, 平时在互联网上通讯, 几乎用不到组播, 但是如果在一个局域网内, 有一条相同的消息需要发送给n个接收端, 这是组播就是相当合适的一个选择了.  
使用**mugglec**, 实现组播的接收是十分方便的, 还是使用上面的[time_serv.c](./time_serv/time_serv.c)作为服务器, 每次时间消息都只会发送一次, 这里我们使用[net_mcast_recv.c](./mcast_recv/net_mcast_recv.c)作为组播的接收者, 读者可以试着开启多个接收者看看实际的效果  

加入组播
```
muggle_socket_t fd = muggle_mcast_join(grp_host, grp_serv, iface, src_grp);
if (fd == MUGGLE_INVALID_SOCKET)
{
	MUGGLE_LOG_ERROR("failed multicast join %s:%s", grp_host, grp_serv);
	exit(EXIT_FAILURE);
}
```

使用**mugglec**加入组播组十分简单, 只需调用`muggle_mcast_join`即可, 而离开组播组则调用`muggle_mcast_leave`  
```
muggle_socket_t muggle_mcast_join(
	const char *host,
	const char *serv,
	const char *iface,
	const char *src_grp);
```
* mcast-IP           加入的组播组ip
* mcast-Port         加入的组播组端口
* net-iface
	* 在*nix上, 此参数填写ifconfig命令结果中的名称
	* 在Windows上, 此参数填写ip地址
* mcast-source-group 过滤组播的源(发送者的ip地址)

### 多线程
到目前位置, 我们所有的消息处理都直接在事件循环的回调函数当中, 这在许多情况下正是我们想要的
* 所有回调函数都在事件循环的同一线程当中, 无需考虑资源竞争, 互斥操作, 降低了代码编写的心智负担
* 避免了跨线程拷贝数据的问题
* 用户还不用考虑`socket上下文`的引用计数问题

但有些时候需要进行一些比较耗时操作, 这时如果都在事件循环的回调函数当中处理就不太合适了.  

[async_serv.c](./async_tcp_serv/async_serv.c)是一个展示多线程处理的例子, 当收到消息时, 会新开启一个工作线程, 并将`socket上下文`传给新的工作线程. 这个例子中需要特别注意的是关于`socket上下文`的引用计数操作  

当要把`socket上下文`传给其他线程使用时, 我们需要增加其引用计数
```
int ref_cnt = muggle_socket_ctx_ref_retain(ctx);
```

在事件循环的释放资源的回调函数中, 依然和之前的例子一样操作即可. 但需要注意的是, 在异步执行的线程当中, 当`socket上下文`不再被使用时, 要扣减其引用计数, 并且当扣减为0时, 需要用户手动执行释放`socket上下文`及其资源的操作
```
static void ctx_release(muggle_socket_context_t *ctx)
{
	user_data_t *user_data = muggle_socket_ctx_get_data(ctx);
	int ref_cnt = muggle_socket_ctx_ref_release(ctx);
	LOG_INFO("context[%s] release, ref_cnt=%d", user_data->str_addr, ref_cnt);
	if (ref_cnt == 0)
	{
		LOG_INFO("context[%s] release", user_data->str_addr);
		free(user_data);

		muggle_socket_ctx_close(ctx);
		free(ctx);
	}
}
```

### 事件管道
有的时候, 我们还需要在另一个线程向 `socket evloop` 中传递消息, 此时可以使用 `muggle_socket_evloop_pipe_t`  
[event_pipe](./event_pipe/event_pipe.c) 是一个展示事件管道的例子  

```
muggle_socket_evloop_pipe_t ev_pipe;
if (muggle_socket_evloop_pipe_init(&ev_pipe) != 0) {
	LOG_ERROR("failed init event pipe");
}

....

muggle_socket_context_t *pipe_reader =
		muggle_socket_evloop_pipe_get_reader(ev_pipe);
muggle_socket_evloop_add_ctx(evloop, pipe_reader);
```
首先通过 `muggle_socket_evloop_pipe_init` 初始化一个事件管道, 接着通过 `muggle_socket_evloop_pipe_get_reader` 获取管道的读端, 并通过 `muggle_socket_evloop_add_ctx` 加入到事件循环当中  

```
void on_message(muggle_event_loop_t *evloop, muggle_socket_context_t *ctx)
{
	MUGGLE_UNUSED(evloop);

	MUGGLE_ASSERT(ctx->sock_type == MUGGLE_SOCKET_CTX_TYPE_PIPE);

	muggle_socket_evloop_pipe_t *ev_pipe =
		(muggle_socket_evloop_pipe_t *)muggle_ev_ctx_data(
			(muggle_event_context_t *)ctx);

	void *data = NULL;
	while ((data = muggle_socket_evloop_pipe_read(ev_pipe)) != NULL) {
		char *s = (char *)data;
		LOG_INFO("on evloop pipe message: %s", s);
		free(s);

		// simulate blocking
		// muggle_msleep(500);
	}
}
```
接下来, 用户可以通过 `muggle_socket_evloop_pipe_write` 向管道中写入数据, 之后会正常触发事件循环的消息回调, 此时的 `ctx->sock_type` 为 `MUGGLE_SOCKET_CTX_TYPE_PIPE` 类型, 其中带的数据就是指向 `muggle_socket_evloop_pipe_t` 的指针  

### 自定义协议
到现在为止, 我们已经看到一些很直观的例子, 当然真实的服务还要处理更多的问题, 比如TCP会遇到粘包, UDP需要自己处理丢包与重传, 还有需要传输自定义的消息而不是单纯的字符串. 下面的例子[foo.c](./foo/foo.c), 我们展示一个自定义协议的TCP服务器/客户端, 其中使用了在[内存模块](../memory/readme_cn.md)中提到的字节缓冲区来收取TCP的消息, 并且自定了消息分发器和编解码处理器.  
那么现在, 让我们从头开始一步一步的构建这个服务/客户端吧

#### 事件循环回调
和上几个小节一样, 首先我们在main当中初始化事件循环, 并指定事件循环的回调函数以及相关的上下文附带数据的结构.  

**TCP Server** [foo_serv_handle.c](./foo/serv/foo_serv_handle.c)  
注册事件循环回调
```
muggle_socket_evloop_handle_init(handle);
muggle_socket_evloop_handle_set_cb_conn(handle, tcp_serv_on_connect);
muggle_socket_evloop_handle_set_cb_msg(handle, tcp_serv_on_message);
muggle_socket_evloop_handle_set_cb_close(handle, tcp_serv_on_close);
muggle_socket_evloop_handle_set_cb_release(handle, tcp_serv_on_release);
muggle_socket_evloop_handle_set_cb_timer(handle, tcp_serv_on_timer);
muggle_socket_evloop_handle_set_timer_interval(handle, 5000);
muggle_socket_evloop_handle_attach(handle, evloop);
```

创建TCP监听socket并加入事件循环中
```
static muggle_thread_ret_t tcp_server_listen(void *p_args)
{
	foo_server_thread_args_t *args = (foo_server_thread_args_t*)p_args;

	// tcp listen
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = muggle_tcp_listen(args->args->host, args->args->port, 512);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_ERROR("failed tcp listen %s %s", args->args->host, args->args->port);
			muggle_msleep(3000);
		}
	} while (fd == MUGGLE_INVALID_SOCKET);

	// new context
	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_LISTEN);

	muggle_socket_evloop_add_ctx(args->evloop, ctx);

	// free arguments
	free(args);

	return 0;
}
```

**TCP Client** [foo_client_handle.c](./foo/client/foo_client_handle.c)  
注册事件循环回调
```
muggle_socket_evloop_handle_init(handle);
muggle_socket_evloop_handle_set_cb_add_ctx(handle, tcp_client_on_add_ctx);
muggle_socket_evloop_handle_set_cb_msg(handle, tcp_client_on_message);
muggle_socket_evloop_handle_set_cb_close(handle, tcp_client_on_close);
muggle_socket_evloop_handle_attach(handle, evloop);
```

创建TCP连接socket并加入事件循环当中
```
static muggle_thread_ret_t tcp_client_connect(void *p_args)
{
	foo_client_thread_args_t *args = (foo_client_thread_args_t*)p_args;

	// tcp connect
	muggle_socket_t fd = MUGGLE_INVALID_SOCKET;
	do {
		fd = muggle_tcp_connect(args->args->host, args->args->port, 3);
		if (fd == MUGGLE_INVALID_SOCKET)
		{
			LOG_ERROR("failed tcp connect %s %s", args->args->host, args->args->port);
			muggle_msleep(3000);
		}
	} while (fd == MUGGLE_INVALID_SOCKET);

	// new context
	muggle_socket_context_t *ctx =
		(muggle_socket_context_t*)malloc(sizeof(muggle_socket_context_t));
	muggle_socket_ctx_init(ctx, fd, NULL, MUGGLE_SOCKET_CTX_TYPE_TCP_CLIENT);

	muggle_socket_evloop_add_ctx(args->evloop, ctx);

	// free arguments
	free(args);

	return 0;
};
```

#### 协议头
由于我们要处理不同类型的消息, 我们需要先定义协议头, 每个消息都带有协议头, 定义如下
```
|<---         head       --->|<---  body   --->|
| message type | body length |    body bytes   |
|    4 bytes   |   4 bytes   | variable length |
```

* 包头前4字节是用户自定义的消息类型
* 包头后4字节, 用于指明body的长度
* body是消息中携带的数据

头定义
```
#pragma pack(push)
#pragma pack(1)

typedef struct foo_msg_header
{
	uint32_t msg_type;
	uint32_t body_len;
}foo_msg_header_t;

#pragma pack(pop)
```

#### 消息
接着, 我们定义消息ID与结构
* 登录消息, 由客户端发起, 服务器回复
* 求和消息, 由客户端发送一组int32型数组列表, 服务器返回相加的结果
* 心跳消息, 由服务器每隔一段时间字段发送的消息

可在[foo_msg.h](./foo/foo/foo_msg.h)中看到消息头与消息的定义

#### 编解码与消息分发
有了消息定义和事件循环回调之后, 我们现在需要考虑接收/发送消息时的编解码(比如处理协议格式, 大小端, 加解密, 压缩/解压等等)以及消息分发. 这里为了展示用法, 我们规定协议头和数据均为为网络字节序, 所以例子中我们分别定义两个编解码器, 分别是:
* 字节流编解码`foo_codec_bytes_t`: 用于转换协议头的大小端, 以及解决TCP的粘包问题
* 大小端编解码`foo_codec_endian_t`: 用于转换消息数据的大小端

接着, 我们初始化消息分发器, 将编解码按照编码的顺序加入消息分发器当中 [foo_handle.c](./foo/foo/foo_handle.c)
```
foo_dispatcher_init(&evloop_data->dispatcher);

foo_codec_endian_t *endian_codec = (foo_codec_endian_t*)malloc(sizeof(foo_codec_endian_t));
foo_codec_endian_init(endian_codec);
foo_dispatcher_append_codec(&evloop_data->dispatcher, (foo_codec_t*)endian_codec);

foo_codec_bytes_t *bytes_codec = (foo_codec_bytes_t*)malloc(sizeof(foo_codec_bytes_t));
foo_codec_bytes_init(bytes_codec);
foo_dispatcher_append_codec(&evloop_data->dispatcher, (foo_codec_t*)bytes_codec);
```

现在我们已经将编解码与消息分发准备就绪, 接着, 我们只需要在初始化的时候注册消息与回调函数的映射关系 [foo.c](./foo/foo.c), 剩下的只需专心处理业务逻辑就可以了.  
```
switch (args.app_type)
{
case APP_TYPE_TCP_SERV:
{
	init_tcp_server_handle(evloop, &handle, &args);

	// register message callbacks
	foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
	foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_REQ_LOGIN, tcp_server_on_req_login);
	foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_PONG, tcp_server_on_msg_pong);
	foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_REQ_SUM, tcp_server_on_req_sum);
}break;
case APP_TYPE_TCP_CLIENT:
{
	init_tcp_client_handle(evloop, &handle, &args);

	// register message callbacks
	foo_dispatcher_t *dispatcher = foo_handle_dipatcher(evloop);
	foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_RSP_LOGIN, tcp_client_on_rsp_login);
	foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_PING, tcp_client_on_msg_ping);
	foo_dispatcher_register(dispatcher, FOO_MSG_TYPE_RSP_SUM, tcp_server_on_rsp_sum);
}break;
default:
{
	LOG_ERROR("invalid app type, exit");
	exit(EXIT_FAILURE);
}break;
}
```

注意: 当前在endian codec中, 我们需要为每一个消息编写独立的大小端处理函数, 这可太无趣了, 那么是否能将这部分代码自动化完成呢. 很可惜, 由于c语言本身没有反射机制, 在不借助外部工具的情况下, 很难做到结构体的自动序列化与反序列化. 好在存在很多开源的项目, 来完成这件事, 比如protobuf, thrift; 或者也可以不使用结构体, 直接使用指定的数据交换格式, 比如json, xml等. 这有些偏离了这篇文章的主题了, 关于自动序列化/反序列化的主题就不在这里展开了.  
