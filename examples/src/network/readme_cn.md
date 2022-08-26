- [Event & Network](#event--network)
	- [事件模块](#事件模块)
		- [事件描述符](#事件描述符)
		- [事件上下文](#事件上下文)
		- [事件循环](#事件循环)
		- [直接使用Event来处理网络消息(不推荐)](#直接使用event来处理网络消息不推荐)
	- [网络模块](#网络模块)
		- [一个简单的Echo服务](#一个简单的echo服务)
		- [Socket事件回调](#socket事件回调)
			- [cb_conn](#cb_conn)
			- [cb_msg](#cb_msg)
			- [cb_close](#cb_close)
			- [cb_release](#cb_release)
			- [cb_add_ctx](#cb_add_ctx)
			- [cb_wake](#cb_wake)
			- [cb_timer](#cb_timer)

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
TODO:

#### cb_wake
TODO:

#### cb_timer
TODO: