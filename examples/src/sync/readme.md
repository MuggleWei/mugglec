- [sync](#sync)
	- [线程](#线程)
		- [创建线程](#创建线程)
		- [detach & join](#detach--join)
	- [原子操作](#原子操作)
		- [原子变量类型](#原子变量类型)
		- [内存序](#内存序)
		- [写入与读取](#写入与读取)
		- [交换](#交换)
		- [CAS](#cas)
	- [引用计数](#引用计数)
	- [互斥锁与条件变量](#互斥锁与条件变量)
	- [自旋锁](#自旋锁)
	- [BlockingQueue](#blockingqueue)
	- [Channel](#channel)
	- [Ring Buffer](#ring-buffer)

# sync
本节讲解**mugglec**中线程, 原子操作以及线程间同步的对象

## 线程

### 创建线程
```
int muggle_thread_create(muggle_thread_t *thread, muggle_thread_routine routine, void *args);
```
* thread: 指向线程对象的指针
* routine: 线程运行的函数
* args: 传给线程的参数

### detach & join
我们看一下例子: [example_thread.c](./thread/example_thread.c)  
首先准备一个线程运行的函数, 它模拟做一个任务
```
muggle_thread_ret_t thread_routine(void *args)
{
	LOG_INFO("start do something");

	muggle_msleep(1000);

	LOG_INFO("end do something");

	return 0;
}
```

当不关心线程是否结束时, 在创建线程后可直接调用`muggle_thread_detach`, 调用之后, 本线程中的代码会继续运行
```
muggle_thread_create(&th, thread_routine, NULL);
muggle_thread_detach(&th);
```

当用户想等待线程结束时, 需要调用`muggle_thread_join`, 调用之后, 本线程会等待指定线程退出
```
muggle_thread_create(&th, thread_routine, NULL);
muggle_thread_join(&th);
```

## 原子操作
**mugglec**提供了一系列原子操作

### 原子变量类型
当前**mugglec**支持4中原子变量类型, 分别是
* muggle_atomic_byte
* muggle_atomic_int
* muggle_atomic_int32
* muggle_atomic_int64

### 内存序
**mugglec**也提供内存序的定义, 所含意义与c11中的一致
* muggle_memory_order_relaxed
* muggle_memory_order_consume
* muggle_memory_order_acquire
* muggle_memory_order_release
* muggle_memory_order_acq_rel
* muggle_memory_order_seq_cst

### 写入与读取
```
muggle_atomic_store(ptr, val, memmodel);
```
* ptr: 指向原子变量的指针
* val: 要赋给原子变量的值
* memmodel: 指定内存序

```
muggle_atomic_load(ptr, memmodel);
```
* ptr: 指向要读取的原子变量
* memmodel: 指定内存序
* 返回值: 返回原子变量当下的值

下面我们看一个经典的内存序的例子: [atomic_load_store.c](./atomic_load_store/atomic_load_store.c)  
线程1: 先将x赋值为1, 再将y赋值为1
```
muggle_atomic_store(args->x, 1, muggle_memory_order_relaxed);
muggle_atomic_store(args->y, 1, muggle_memory_order_release);
```
线程2: 先循环读取y的值, 当y值为1时才去读取x的值
```
while (!(y = muggle_atomic_load(args->y, muggle_memory_order_acquire)));
x = muggle_atomic_load(args->x, muggle_memory_order_relaxed);
```
此时我们使用了`acquire-release`序来确保在当线程2读取到y值为1时, 在y值之前被赋值的x的值也一定会被线程2正确的读取到

### 交换
```
muggle_atomic_exchange(ptr, val, memmodel);
muggle_atomic_exchange32(ptr, val, memmodel);
muggle_atomic_exchange64(ptr, val, memmodel);
```
* ptr: 指向原子变量的指针
* val: 将要赋予原子变量的值
* memmodel: 指定内存序
* 返回值: 原本存储在原子变量当中的值

### CAS
比较并交换(compare and swap, CAS)是一种常见的原子操作
```
muggle_atomic_cmp_exch_weak(ptr, expected, desired, memmodel);
muggle_atomic_cmp_exch_weak32(ptr, expected, desired, memmodel);
muggle_atomic_cmp_exch_weak64(ptr, expected, desired, memmodel);

muggle_atomic_cmp_exch_strong(ptr, expected, desired, memmodel);
muggle_atomic_cmp_exch_strong32(ptr, expected, desired, memmodel);
muggle_atomic_cmp_exch_strong64(ptr, expected, desired, memmodel);
```
* ptr: 指向原子变量的指针
* expected: 用于与`*ptr`对比的值的指针
* desired: 对比相等时赋给`*ptr`的值
* memmode: 内存序

对比`*ptr`与`*expected`的值    
* 若相等, 则将`desired`的值赋给`*ptr`
* 若不相等, 则将`*ptr`的值写入`*expected`当中

当成功时, 返回true, 否则返回false. `_weak`的函数可能会出现虚假失败的情况.  

例子 [atomic_cas.c](./atomic_cas/atomic_cas.c) 展示了CAS的作为引用计数的用法:
```
int ref_cnt_release(muggle_atomic_int *val)
{
	muggle_atomic_int v = 0, desired=0;
	do {
		v = *val;
		if (v == 0)
		{
			return -1;
		}
		desired = v - 1;
	} while (!muggle_atomic_cmp_exch_strong(val, &v, desired, muggle_memory_order_relaxed));

	return desired;
}
```
函数`ref_cnt_release`将引用计数减1, 当引用计数已经为0的情况下, 直接返回-1

## 引用计数
上一小节的例子出现了引用计数自减的例子, 其实**mugglec**中已经提供了一个引用计数对象: `muggle_ref_cnt_t`

`muggle_ref_cnt_init`将引用计数初始化为想要的值, 一般情况下, 指定的初始值为1
```
int muggle_ref_cnt_init(muggle_ref_cnt_t *ref, int init_val);
```

通过`muggle_ref_cnt_retain`和`muggle_ref_cnt_release`实现引用计数的自增/自减操作
```
int muggle_ref_cnt_retain(muggle_ref_cnt_t *ref);
int muggle_ref_cnt_release(muggle_ref_cnt_t *ref);
```
注意, 引用计数总是要大等于0的, 当引用计数到0之后, 无论是`retain`还是`release`都将失败并返回-1


## 互斥锁与条件变量
**mugglec**提供了互斥锁(`muggle_mutex_t`)以及条件变量(`muggle_condition_variable_t`)  
例子 [mutex_cv.c](./mutex_cv/mutex_cv.c) 展示了一个简单的生产者-消费者的程序, 使用了互斥锁和条件变量, 同一时间只能有一条线程在访问临界区

## 自旋锁
除了互斥锁之外, **mugglec**还提供了自旋锁  
例子 [example_spinlock.c](./spinlock/example_spinlock.c) 展示了使用自旋锁来实现一个简单的生产者-消费者的程序  

## BlockingQueue
上面使用互斥锁/自旋锁来实现了简单的生产者-消费者的程序, **mugglec**当中还提供了方便用于不同线程交换数据的对象. `muggle_array_blocking_queue_t`是一个线程安全的阻塞队列, 当队列满时, `put`操作将阻塞; 当队列空时, `take`操作将阻塞. 我们使用它来实现一个生产者-消费者的例子 [blocking_queue.c](./blocking_queue/blocking_queue.c)  

生产者: 往`muggle_array_blocking_queue_t`中`put`目标数量的数据, 生产者线程退出时, 推送一个`block_id`为-1的数据
```
for (int i = 0; i < args->target_cnt; i++)
{
	block_data_t *data = (block_data_t*)malloc(sizeof(block_data_t));
	data->thread_id = args->id;
	data->block_id = i;

	muggle_array_blocking_queue_put(queue, (void*)data);
}

block_data_t *data = (block_data_t*)malloc(sizeof(block_data_t));
data->thread_id = args->id;
data->block_id = -1;
muggle_array_blocking_queue_put(queue, (void*)data);
```

消费者: 从`muggle_array_blocking_queue_t`中`take`数据, 当收到生产者退出数据的数量和生产者数量相等时, 消费者退出
```
while (1)
{
	block_data_t *data = muggle_array_blocking_queue_take(&queue);
	if (data->block_id == -1)
	{
		LOG_INFO("recv [%d] exit data", data->thread_id);
		completed_cnt++;
		if (completed_cnt == PRODUCER_NUM)
		{
			break;
		}
	}
	else
	{
		LOG_INFO("consume #%d[%d]", data->block_id, data->thread_id);
		free(data);
	}
}
```

## Channel
除了`BlockingQueue`, **mugglec**还提供了几个特殊的用于线程交换数据的对象.  
`muggle_channel_t`是一个特殊的管道, 它用于线程间交换数据的效率要高于`muggle_array_blocking_queue_t`, 但是**用户需要保证同时间最多只有1个消费者在从管道中读取数据**. 当`muggle_channel_t`管道满时, 不会阻塞, 而是返回错误消息`MUGGLE_ERR_FULL`; 而当`muggle_channel_t`管道为空时, 读取的表现为等待  
`muggle_channel_t`在初始化时, 可以选择不同的读/写模式:
```
// 写模式
MUGGLE_CHANNEL_FLAG_WRITE_MUTEX,  //!< write lock use mutex
MUGGLE_CHANNEL_FLAG_WRITE_FUTEX,  //!< write lock use futex if be supported
MUGGLE_CHANNEL_FLAG_WRITE_SPIN,   //!< write lock use spinlock
MUGGLE_CHANNEL_FLAG_WRITE_SINGLE, //!< user guarantee only one writer use this channel

// 读模式
MUGGLE_CHANNEL_FLAG_READ_FUTEX, //!< reader with futex wait if supported
MUGGLE_CHANNEL_FLAG_READ_MUTEX, //!< reader with mutex with
MUGGLE_CHANNEL_FLAG_READ_BUSY,  //!< reader busy loop until read message from channel

int muggle_channel_init(muggle_channel_t *chan, muggle_atomic_int capacity, int flags);
```
* `chan`: 指向channel的指针
* `capacity`: channel的容量大小
* `flag`: 上面提到的读/写模式的位或操作结果, 当用户没有特殊需求时, 建议直接填入0

现在我们修改一下上一小节的程序, 改为使用`muggle_channel_t`来实现生产者-消费者. 由于我们明确知道只有一个消费者, 所以使用`muggle_channel_t`是一个很好的选择 [chan.c](./chan/chan.c)  
生产者: 往`muggle_channel_t`中`write`目标数量的数据, 生产者线程退出时, 推送一个`block_id`为-1的数据
```
for (int i = 0; i < args->target_cnt; i++)
{
	block_data_t *data = (block_data_t*)malloc(sizeof(block_data_t));
	data->thread_id = args->id;
	data->block_id = i;

	if (muggle_channel_write(chan, data) == MUGGLE_ERR_FULL)
	{
		LOG_WARNING("failed write #%d[%d], channel full!", data->block_id, data->thread_id);
		free(data);
	}
}

block_data_t *data = (block_data_t*)malloc(sizeof(block_data_t));
data->thread_id = args->id;
data->block_id = -1;
while (muggle_channel_write(chan, (void*)data) == MUGGLE_ERR_FULL);
```

消费者: 从`muggle_channel_t`中`read`数据, 当收到生产者退出数据的数量和生产者数量相等时, 消费者退出
```
while (1)
{
	block_data_t *data = muggle_channel_read(&chan);
	if (data->block_id == -1)
	{
		LOG_INFO("recv [%d] exit data", data->thread_id);
		completed_cnt++;
		if (completed_cnt == PRODUCER_NUM)
		{
			break;
		}
	}
	else
	{
		LOG_INFO("consume #%d[%d]", data->block_id, data->thread_id);
		free(data);
	}
}
```

## Ring Buffer
`muggle_ring_buffer_t`是**mugglec**当中另一种特殊的用于线程交换数据的对象, 默认情况下, **支持所有消费者去读取每一条消息, 但是需要注意, 当读取过慢导致被生产者"套圈", 这会导致消费者丢失"一圈"的消息**  

[ring_buf.c](./ring_buf/ring_buf.c) 是一个使用`muggle_ring_buffer_t`的例子, 在本例子中, 1个生产者将消息广播给了4个消费者  
生产者
```
block_data_t datas[16];
const int len = sizeof(datas) / sizeof(datas[0]);
int i = 0;
for (i = 0; i < 32; i++)
{
	int idx = i % len;
	datas[idx].block_id = i;
	muggle_ring_buffer_write(&ring, &datas[idx]);
	muggle_msleep(5);
}
```
消费者
```
while (1)
{
	block_data_t *data = (block_data_t*)muggle_ring_buffer_read(ring, rpos++);
	if (data->block_id == -1)
	{
		break;
	}

	LOG_INFO("consumer[%d] recv data %d", args->id, data->block_id);
	++cnt;
}
```

在例子当中, 我们将`muggle_ring_buffer_t`的容量初始化为16, 小于要发的消息的大小32, 而在生产者发送消息的过程中, 每发送一个消息都sleep了5毫秒. 可以试着将这行代码注释掉, 有概率将会观察到, 有些消费者最终收到的消息条目数是16, 被生产者套圈的结果是丢失了"一整圈"的消息  